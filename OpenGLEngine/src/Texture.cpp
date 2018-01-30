#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include "ErrorHandling.h"

//REFERENCE: Largely inspried by Dr Anton Geraldans book - Anton's OpenGL 4 Tutorials
namespace scene {
  void Texture::Bind()
  {
    GLCall(glBindTexture(GL_TEXTURE_2D, texture_id_));
  }

  void Texture::Load(char* filename)
  {
    filename_ = filename;
    int x, y, n;
    int force_channels = 4;
    unsigned char* image_data = stbi_load(filename, &x, &y, &n, force_channels);
    if (!image_data) {
      fprintf(stderr, "Error: could not load %s\n", filename);
    }
  }

  Texture::Texture()
  {
   
  }

  Texture::Texture(char* filename)
  {
    GLCall(glGenTextures(1, &texture_id_));
    filename_ = filename;
  }

  Texture::~Texture()
  {
    GLCall(glDeleteTextures(1, &texture_id_));
  }
} //namespace scene
