#pragma once
#include <memory>
#include <vector>
#include "VertexGrid.h"

namespace scene {
  class Object;
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
    std::shared_ptr<Shader> scene_shader_;
    std::unique_ptr<VertexGrid> vertex_grid_;

    void Init(bool should_shadow_map);
  public:
    //Blank constructor
    inline CausticMapping() {}
    //Creates the necessary framebuffers and textures
    inline CausticMapping(const int* window_width, const int* window_height, bool should_shadow_map)
      : window_width_(window_width), window_height_(window_height), should_shadow_map_(should_shadow_map)
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
    void LoadShaders();
  };
}

