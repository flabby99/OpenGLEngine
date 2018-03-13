#include "FrameBuffer.h"
#include "Texture.h"

namespace render
{
  FrameBuffer::FrameBuffer()
  {
    GLCall(glGenFramebuffers(1, &renderer_id_));
  }


  FrameBuffer::~FrameBuffer()
  {
    GLCall(glDeleteFramebuffers(1, &renderer_id_));
  }

  void FrameBuffer::AttachTexture(std::shared_ptr<scene::Texture> texture, GLenum attachment)
  {
    //0 indicates attaching to mipmap level 0
    Bind();
    GLCall(glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture->GetID(), 0));
    draw_targets_.push_back(attachment);
  }
}
