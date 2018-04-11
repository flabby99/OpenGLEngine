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
      auto shadow_texture = std::make_shared<scene::DepthTexture>(*window_width_, *window_height_, GL_TEXTURE_2D);
      shadow_texture->SetSlot(GL_TEXTURE4);
      shadow_map_->AttachTexture(shadow_texture, GL_DEPTH_ATTACHMENT);
      shadow_map_->BufferStatusCheck();
    }
    //Load the point sprite texture
    const char* sprite_location = "res/Models/textures/star.png";
    point_sprite_ = std::make_unique<scene::Texture>();
    point_sprite_->LoadNoMip(sprite_location);
    point_sprite_->SetSlot(GL_TEXTURE4);
    //Create the shaders
    LoadShaders();
    //Create a vertex grid of predifined size
    vertex_grid_ = std::make_unique<VertexGrid>(*window_width_, *window_height_);
    query_ = std::make_unique<Query>(GL_SAMPLES_PASSED);
    //persp_proj_ = glm::perspective(glm::radians(45.0f), (float)*window_width_ / (float)*window_height_, 0.1f, 80.0f);
    persp_proj_ = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 20.0f);
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
    //For directional lights, this position is really the inverse of the direction
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
      receiver_shader_->SetUniform4fv("view", light_view_matrix_);
      receiver_shader_->SetUniform4fv("proj", persp_proj_);
      for (auto object : receivers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        receiver_shader_->SetUniform4fv("model", object->GetGlobalModelMatrix());
        render::Renderer::Draw(*object);
      }

      //Visualise to see what I'm getting
     /* receiver_positions_->Unbind();
      render::Renderer::SetScreenAsRenderTarget();
      glViewport(*window_width_ / 2, *window_height_ / 2, *window_width_  / 2, *window_height_ / 2);
      Visualise(receiver_positions_, 0, post_process, ss_quad);*/
    }
    
    //Obtain 3D positions and surface normals of the producers
    {
      query_->Begin();
      caustic_pos_norms_->SetBufferForDraw();
      glViewport(0, 0, *window_width_, *window_height_);
      render::Renderer::ClearColourOnly();
      producer_shader_->Bind();
      producer_shader_->SetUniform4fv("view", light_view_matrix_);
      producer_shader_->SetUniform4fv("proj", persp_proj_);
      for (auto object : producers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        producer_shader_->SetUniform4fv("model", object->GetGlobalModelMatrix());
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
      //Combine the flux intensity
      //This works but it is a synchronus operation
     /* query_->Result(&pixels_renderered_last_frame);*/
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      point_sprite_->Bind();
      caustic_pos_norms_->GetTexture(0)->Bind();
      caustic_pos_norms_->GetTexture(1)->Bind();
      receiver_positions_->GetTexture(0)->Bind();
      glm::vec3 light_direction = -glm::normalize(light_position_);
      float visible = 1.f - (float)(pixels_renderered_last_frame_) / (float)(*window_width_ * *window_height_);
      if (visible < 0.01f) visible = 0.01f;
      //TODO figure out correct value for this
      //TODO one option would be make it variable with a key press and show the difference
      //TODO same for num iterations in intersection estimation
      visible *= 0.1f;
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
      shadow_map_->SetDepthBufferForDraw();
      glViewport(0, 0, *window_width_, *window_height_);
      render::Renderer::Clear();
      shadow_shader_->Bind();
      shadow_shader_->SetUniform4fv("view_proj", persp_proj_ * light_view_matrix_);
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2.0f, 4.0f);
      // Draw from the light's point of view

      for (auto object : producers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        shadow_shader_->SetUniform4fv("model", object->GetGlobalModelMatrix());
        render::Renderer::Draw(*object);
      }
      for (auto object : receivers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        shadow_shader_->SetUniform4fv("model", object->GetGlobalModelMatrix());
        render::Renderer::Draw(*object);
      }
      glDisable(GL_POLYGON_OFFSET_FILL);
      render::Renderer::SetScreenAsRenderTarget();
      shadow_map_->Unbind();
      glViewport(*window_width_ / 2, *window_height_ / 2, *window_width_ / 2, *window_height_ / 2);
      Visualise(shadow_map_, 0, post_process, ss_quad);
    }
  }
  void CausticMapping::LoadShaders()
  {
    const std::string caustic_shaders = "config/causticshaders.txt";
    receiver_shader_ = std::make_shared<Shader>("receiver", caustic_shaders);
    producer_shader_ = std::make_shared<Shader>("producer", caustic_shaders);
    caustic_shader_ = std::make_shared<Shader>("caustic", caustic_shaders);
    if (should_shadow_map_) {
      shadow_shader_ = std::make_shared<Shader>("shadow", caustic_shaders);
    }
  }
  void CausticMapping::BindCausticTexture() {
    caustic_map_->GetTexture(0)->Bind();
  }
  void CausticMapping::BindReceiverTexture() {
    receiver_positions_->GetTexture(0)->Bind();
  }
  void CausticMapping::BindShadowTexture() {
    shadow_map_->GetTexture(0)->Bind();
  }

}