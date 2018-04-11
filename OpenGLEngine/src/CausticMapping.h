#pragma once
#include <memory>
#include <vector>
#include "VertexGrid.h"
#include "Query.h"

namespace scene {
  class Object;
  class Texture;
}

namespace render
{
  class FrameBuffer;
  class Shader;
  //class VertexGrid;
  class CausticMapping
  {
  private:
    const int* window_width_;
    const int* window_height_;
    bool should_shadow_map_;
    std::shared_ptr<FrameBuffer> receiver_positions_;
    std::shared_ptr<FrameBuffer> caustic_pos_norms_;
    std::shared_ptr<FrameBuffer> caustic_map_;
    std::shared_ptr<FrameBuffer> shadow_map_;
    std::shared_ptr<Shader> receiver_shader_;
    std::shared_ptr<Shader> producer_shader_;
    std::shared_ptr<Shader> caustic_shader_;
    std::shared_ptr<Shader> shadow_shader_;
    std::unique_ptr<VertexGrid> vertex_grid_;
    std::unique_ptr<Query> query_;
    std::unique_ptr<scene::Texture> point_sprite_;

    bool past_first_frame = false;
    glm::vec3 light_position_;
    glm::vec3 origin_ = glm::vec3(0.0f);
    glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 light_view_matrix_;
    glm::mat4 persp_proj_;
    GLuint pixels_renderered_last_frame_;

    void Init(bool should_shadow_map);
  public:
    //Blank constructor
    inline CausticMapping() {}
    //Creates the necessary framebuffers and textures
    inline CausticMapping(const int* window_width, const int* window_height, bool should_shadow_map, glm::vec3 light_position )
      : window_width_(window_width), window_height_(window_height), should_shadow_map_(should_shadow_map),
        light_position_(light_position)
    {
      Init(should_shadow_map_);
    }
    inline ~CausticMapping() {}
    //Renders the indexed texture in the framebuffer to the screen
    void Visualise(std::shared_ptr<FrameBuffer> fb, unsigned int texture_index, Shader* post_process, scene::Object* ss_quad);
    //Calculate a caustic map for a receiver set and producer set of objects
    void CalculateCaustics(std::vector<std::shared_ptr<scene::Object>> receivers,
      std::vector<std::shared_ptr<scene::Object>> producers,
      render::Shader* post_process, scene::Object* ss_quad);
    void BindCausticTexture();
    void BindReceiverTexture();
    void BindShadowTexture();
    void LoadShaders();
    void SetLightPosition(const glm::vec3& light_position) {
      light_position_ = light_position;
    }
    inline glm::mat4 getLightViewProj() {
      return persp_proj_ * light_view_matrix_;
    }
  };
}

