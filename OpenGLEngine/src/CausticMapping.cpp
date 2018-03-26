#define GLM_ENABLE_EXPERIMENTAL
#include "CausticMapping.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Renderer.h"
#include <string>
#include "Shader.h"
#include "glm\glm.hpp"
#include "glm/gtx/transform.hpp"


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
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    //TODO make this a changeable
    glm::vec3 light_position = glm::vec3(0.0f, 4.0f, 20.0f);
    glm::vec3 origin = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 light_view_matrix = glm::lookAt(light_position, origin, up);
    glm::mat4 persp_proj = glm::perspective(glm::radians(45.0f), (float)*window_width_ / (float)*window_height_, 0.1f, 300.0f);

    //Obtain 3D world positions of the receiver geometry
    {
      //render the scene from the light's point of view and render co ords to texture
      receiver_positions_->SetBufferForDraw();
      render::Renderer::Clear();
      receiver_shader_->Bind();
      for (auto object : receivers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        receiver_shader_->SetUniform4fv("model", object->GetGlobalModelMatrix());
        receiver_shader_->SetUniform4fv("view", light_view_matrix);
        receiver_shader_->SetUniform4fv("proj", persp_proj);
        render::Renderer::Draw(*object);
      }

      //Visualise to see what I'm getting
      //Visualise(receiver_positions_, 0, post_process, ss_quad);
    }
    
    //Obtain 3D positions and surface normals of the producers
    {
      caustic_pos_norms_->SetBufferForDraw();
      render::Renderer::Clear();
      producer_shader_->Bind();
      for (auto object : producers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        producer_shader_->SetUniform4fv("model", object->GetGlobalModelMatrix());
        producer_shader_->SetUniform4fv("view", light_view_matrix);
        producer_shader_->SetUniform4fv("proj", persp_proj);
        render::Renderer::Draw(*object);
      }
      //Set temp slot to the current slot, set current slot to zero and then set back the slot after visualising
      //Visualise(caustic_pos_norms_, 1, post_process, ss_quad);
    }
    //Create a caustic map texture
    {
      caustic_map_->SetBufferForDraw();
      render::Renderer::Clear();
      caustic_shader_->Bind();
      caustic_pos_norms_->GetTexture(0)->Bind();
      caustic_pos_norms_->GetTexture(1)->Bind();
      receiver_positions_->GetTexture(0)->Bind();
      glm::vec3 light_direction = -glm::normalize(light_position);
      caustic_shader_->SetUniform3f("light_direction", light_direction);
      caustic_shader_->SetUniform4fv("view_proj", persp_proj * light_view_matrix);
      render::Renderer::DrawPoints(vertex_grid_.get());
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
}