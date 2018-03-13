#pragma once
#include "glew\glew.h"
#include "ErrorHandling.h"
#include <vector>
#include <memory>

namespace scene {
  class Texture;
}

namespace render
{
  class FrameBuffer
  {
  public:
    FrameBuffer();
    ~FrameBuffer();
    inline void Bind() const {
      GLCall(glBindFramebuffer(GL_FRAMEBUFFER, renderer_id_));
    }
    inline void Unbind() const {
      GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    //Attaches a texture to the frame buffer and adds to the list of enums
    //@param attachment - the place to attacch
    //@param texture - the texture to attach
    void AttachTexture(std::shared_ptr<scene::Texture> texture, GLenum attachment);

    //NOTE could also add support for renderbuffers

  private:
    GLuint renderer_id_;
    //A vector of GLenums which are the places to render to
    std::vector<GLenum> draw_targets_;
  };
}

