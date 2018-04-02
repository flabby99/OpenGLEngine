#define GLM_ENABLE_EXPERIMENTAL
#include <string>
#include "CausticMapping.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "glm\glm.hpp"
#include "glm/gtx/transform.hpp"
#include <iostream>


namespace render
{
  void CausticMapping::Init(bool should_shadow_map)
  {
    //Create the frame buffers
    {
      receiver_positions_ = std::make_shared<render::FrameBuffer>();
      auto positions_texture = std::make_shared<scene::Texture>(*window_width_, *window_height_, GL_TEXTURE_2D);
      positions_texture->SetSlot(GL_TEXTURE2);
      receiver_positions_->AttachTexture(positions_texture, GL_COLOR_ATTACHMENT0);
      receiver_positions_->BufferStatusCheck();
    }

    //Could add a depth map into this
    {
      caustic_pos_norms_ = std::make_shared<render::FrameBuffer>();
      auto positions_texture = std::make_shared<scene::Texture>(*window_width_, *window_height_, GL_TEXTURE_2D);
      caustic_pos_norms_->AttachTexture(positions_texture, GL_COLOR_ATTACHMENT0);
      auto normals_texture = std::make_shared<scene::Texture>(*window_width_, *window_height_, GL_TEXTURE_2D);
      normals_texture->SetSlot(GL_TEXTURE1);
      caustic_pos_norms_->AttachTexture(normals_texture, GL_COLOR_ATTACHMENT1);
      caustic_pos_norms_->BufferStatusCheck();
    }

    {
      caustic_map_ = std::make_shared<render::FrameBuffer>();
      auto map_texture = std::make_shared<scene::Texture>(*window_width_, *window_height_, GL_TEXTURE_2D);
      map_texture->SetSlot(GL_TEXTURE3);
      caustic_map_->AttachTexture(map_texture, GL_COLOR_ATTACHMENT0);
      caustic_map_->BufferStatusCheck();
    }

    if (should_shadow_map_)
    {
      shadow_map_ = std::make_shared<render::FrameBuffer>();
      auto map_texture = std::make_shared<scene::Texture>(*window_width_, *window_height_, GL_TEXTURE_2D);
      shadow_map_->AttachTexture(map_texture, GL_COLOR_ATTACHMENT0);
      shadow_map_->BufferStatusCheck();
    }
    //Create the shaders
    LoadShaders();
    //Create a vertex grid of predifined size
    vertex_grid_ = std::make_unique<VertexGrid>(*window_width_, *window_height_);
    query_ = std::make_unique<Query>(GL_SAMPLES_PASSED);
    persp_proj_ = glm::perspective(glm::radians(45.0f), (float)*window_width_ / (float)*window_height_, 0.1f, 300.0f);
    pixels_renderered_last_frame_ = *window_height_ * *window_width_;
  }

  void CausticMapping::Visualise( std::shared_ptr<FrameBuffer> fb, unsigned int texture_index,
                                  render::Shader* post_process, scene::Object* ss_quad) 
  {
    std::shared_ptr<scene::Texture> current_texture;

    fb->Unbind();
    current_texture = fb->GetTexture(texture_index);
    GLenum temp_slot = current_texture->GetSlot();
    current_texture->SetSlot(GL_TEXTURE0);
    current_texture->Bind();
    post_process->Bind();
    render::Renderer::Draw(*ss_quad);
    current_texture->SetSlot(temp_slot);
  }

  void CausticMapping::CalculateCaustics(std::vector<std::shared_ptr<scene::Object>> receivers,
    std::vector<std::shared_ptr<scene::Object>> producers,
    render::Shader* post_process, scene::Object* ss_quad) {
    glClearColor(0.f, 0.f, 0.0f, 0.0f);
    light_view_matrix_ = glm::lookAt(light_position_, origin_, up_);
    if (past_first_frame) {
      query_->ResultNoWait(&pixels_renderered_last_frame_);
    }
    past_first_frame = true;

    //Obtain 3D world positions of the receiver geometry
    {
      //render the scene from the light's point of view and render co ords to texture
      receiver_positions_->SetBufferForDraw();
      glViewport(0, 0, *window_width_, *window_height_);
      render::Renderer::Clear();
      receiver_shader_->Bind();
      for (auto object : receivers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        receiver_shader_->SetUniform4fv("model", object->GetGlobalModelMatrix());
        receiver_shader_->SetUniform4fv("view", light_view_matrix_);
        receiver_shader_->SetUniform4fv("proj", persp_proj_);
        render::Renderer::Draw(*object);
      }

      //Visualise to see what I'm getting
      receiver_positions_->Unbind();
      render::Renderer::SetScreenAsRenderTarget();
      glViewport(*window_width_ / 2, *window_height_ / 2, *window_width_  / 2, *window_height_ / 2);
      Visualise(receiver_positions_, 0, post_process, ss_quad);
    }
    
    //Obtain 3D positions and surface normals of the producers
    {
      query_->Begin();
      caustic_pos_norms_->SetBufferForDraw();
      glViewport(0, 0, *window_width_, *window_height_);
      render::Renderer::ClearColourOnly();
      producer_shader_->Bind();
      for (auto object : producers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        producer_shader_->SetUniform4fv("model", object->GetGlobalModelMatrix());
        producer_shader_->SetUniform4fv("view", light_view_matrix_);
        producer_shader_->SetUniform4fv("proj", persp_proj_);
        render::Renderer::Draw(*object);
      }
      query_->End();
      //Set temp slot to the current slot, set current slot to zero and then set back the slot after visualising
      caustic_pos_norms_->Unbind();
      render::Renderer::SetScreenAsRenderTarget();
      glViewport(0, 0, *window_width_ / 2, *window_height_ / 2);
      Visualise(caustic_pos_norms_, 0, post_process, ss_quad);
    }

    //Create a caustic map texture
    {
      caustic_map_->SetBufferForDraw();
      glViewport(0, 0, *window_width_, *window_height_);
      render::Renderer::Clear();
      caustic_shader_->Bind();
      glPointSize(1);
      //Combine the flux intensity
      //This works but it is a synchronus operation
     /* query_->Result(&pixels_renderered_last_frame);*/
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      caustic_pos_norms_->GetTexture(0)->Bind();
      caustic_pos_norms_->GetTexture(1)->Bind();
      receiver_positions_->GetTexture(0)->Bind();
      glm::vec3 light_direction = -glm::normalize(light_position_);
      float visible = 1.f - (float)(pixels_renderered_last_frame_) / (float)(*window_width_ * *window_height_);
      if (visible < 0.01f) visible = 0.01f;
      caustic_shader_->SetUniform1f("surface_area", visible);
      caustic_shader_->SetUniform3f("light_direction", light_direction);
      caustic_shader_->SetUniform4fv("view_proj", persp_proj_ * light_view_matrix_);
      render::Renderer::DrawPoints(vertex_grid_.get());
      glDisable(GL_BLEND);

      render::Renderer::SetScreenAsRenderTarget();
      caustic_map_->Unbind();
      glViewport(0, *window_height_ / 2, *window_width_ / 2, *window_height_ / 2);
      Visualise(caustic_map_, 0, post_process, ss_quad);
    }

    //Optionally Construct a shadow map
    if (should_shadow_map_) 
    {
      shadow_map_->SetBufferForDraw();
    }
  }
  void CausticMapping::LoadShaders()
  {
    const std::string caustic_shaders = "config/causticshaders.txt";
    receiver_shader_ = std::make_shared<Shader>("receiver", caustic_shaders);
    producer_shader_ = std::make_shared<Shader>("producer", caustic_shaders);
    caustic_shader_ = std::make_shared<Shader>("caustic", caustic_shaders);
  }
  void CausticMapping::BindCausticTexture() {
    caustic_map_->GetTexture(0)->Bind();
  }
  void CausticMapping::BindReceiverTexture() {
    receiver_positions_->GetTexture(0)->Bind();
  }

}