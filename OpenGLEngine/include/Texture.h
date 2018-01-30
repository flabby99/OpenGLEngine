#pragma once
#include "glew/glew.h"
#include <string>

namespace scene {
  class Texture {
    GLuint texture_id_;
    char* filename_;

  public:
    void Bind();
    void Load(char* filename);
    Texture();
    Texture(char* filename);
    ~Texture();
    inline char* GetFileName() const { return filename_; }
  };
} //namespace scene