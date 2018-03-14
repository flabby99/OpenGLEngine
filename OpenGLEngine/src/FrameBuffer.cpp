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
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->GetID(), 0));
    draw_targets_.push_back(attachment);
    textures_.push_back(texture);
  }
  void FrameBuffer::SetBufferForDraw()
  {
    Bind();
    GLCall(glDrawBuffers(draw_targets_.size(), &draw_targets_[0]));
  }
  bool FrameBuffer::BufferStatusCheck()
  {
    Bind();
    GLCall(GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (GL_FRAMEBUFFER_COMPLETE != status) {
      fprintf(stderr, "ERROR: incomplete framebuffer\n");
      return 0;
    }
    return 1;
  }
}
