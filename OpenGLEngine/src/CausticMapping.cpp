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
    {
      const std::string caustic_shaders = "config/causticshaders.txt";
      receiver_shader_ = std::make_shared<Shader>("receiver", caustic_shaders);
      //TODO put back in
    /*  producer_shader_ = std::make_shared<Shader>("producer", caustic_shaders);
      caustic_shader_ = std::make_shared<Shader>("caustic", caustic_shaders);
      shadow_shader_ = std::make_shared<Shader>("shadow", caustic_shaders);*/
    }
  }

  void CausticMapping::Visualise( std::shared_ptr<FrameBuffer> fb, unsigned int texture_index,
                                  render::Shader* post_process, scene::Object* ss_quad) 
  {
    fb->Unbind();
    fb->GetTexture(0)->Bind();
    post_process->Bind();
    render::Renderer::Draw(*ss_quad);
  }

  void CausticMapping::CalculateCaustics(std::vector<std::shared_ptr<scene::Object>> receivers,
    std::vector<std::shared_ptr<scene::Object>> producers,
    render::Shader* post_process, scene::Object* ss_quad) {
    //TODO make this a changeable
    glm::vec3 light_position = glm::vec3(0.0f, 0.0f, 20.0f);
    glm::vec3 origin = glm::vec3(0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 light_view_matrix = glm::lookAt(light_position, origin, up);
    //TODO - do I need the projection?
    //glm::mat4 light_projection_matrix = 
      //glm::perspective(glm::radians(45.0f), (float)*window_width_ / (float)*window_height_, 0.1f, 300.0f);
    //Obtain 3D positions of the receiver geometry
    {
      receiver_positions_->SetBufferForDraw();
      //render the scene from the light's point of view and render co ords to texture
      receiver_shader_->Bind();
      //Use this shader
      for (auto object : receivers) {
        glm::mat4 model_matrix = object->GetGlobalModelMatrix();
        glm::mat4 light_mv = light_view_matrix * model_matrix;
        receiver_shader_->SetUniform4fv("model_view_matrix", light_mv);
        render::Renderer::Draw(*object);
      }

      //Visualise to see what I'm getting
      Visualise(receiver_positions_, 0, post_process, ss_quad);
      //TODO call this in main
    }
    //Obtain 3D positions and surface normals of the producers
    {
      caustic_pos_norms_->SetBufferForDraw();
    }
    //Create a caustic map texture
    {
      caustic_map_->SetBufferForDraw();
    }
    //Optionally Construct a shadow map
    if (should_shadow_map_) 
    {
      shadow_map_->SetBufferForDraw();
    }
  }
}