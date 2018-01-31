#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include "ErrorHandling.h"

//REFERENCE: Largely inspried by Dr Anton Geraldans book - Anton's OpenGL 4 Tutorials
namespace scene {
  void Texture::Bind()
  {
    GLCall(glBindTexture(type_, texture_id_));
  }

  void Texture::Load(char* filename)
  {
    int x, y;
    filename_ = filename;
    type_ = GL_TEXTURE_2D;
    unsigned char *image_data = LoadTexImage(filename, &x, &y);
    //Copy image data into the openGL texture
    GLCall(glGenTextures(1, &texture_id_));
    GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture_id_));
    //TODO, think of what will happen if we are reading a RGB image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    //Good for anti-aliasing and safe wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    stbi_image_free(image_data);
  }

  unsigned char* Texture::LoadTexImage(char * filename, int* x, int* y)
  {
      int n; //x is width y is height
      int force_channels = 4;
      unsigned char* image_data = stbi_load(filename, x, y, &n, force_channels);
      if (!image_data) {
          fprintf(stderr, "Error: could not load %s\n", filename);
          exit(-1);
      }
      //Check if the image is has dimensions which are a power of two
      if ((*x & (*x - 1) != 0) || *y & (*y - 1) != 0) {
          fprintf(stderr, "WARNING: texture %s is not power of 2 dimensions\n", filename);
      }
      //Flip the image - openGL expects 0 on the Y-axis to be at the bottom of the texture
      //Do this by swapping the top and bottom halves
      int width_in_bytes = *x * 4;
      unsigned char* top = NULL;
      unsigned char* bottom = NULL;
      unsigned char temp = 0;
      int half_height = *y / 2;

      for (int row = 0; row < half_height; ++row) {
          top = image_data + row * width_in_bytes;
          bottom = image_data + (*y - row - 1) * width_in_bytes;
          for (int col = 0; col < width_in_bytes; ++col) {
              temp = *top;
              *top = *bottom;
              *bottom = temp;
              top++;
              bottom++;
          }
      }
      return image_data;
  }

  void Texture::CreateCubeMap(const char * front, const char * back, const char * top, const char * bottom, const char * left, const char * right)
  {
      type_ = GL_TEXTURE_CUBE_MAP;
      GLCall(glGenTextures(1, &texture_id_));
      GLCall(glActiveTexture(GL_TEXTURE0));
      GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_));
      //Load each cube map side
      assert(LoadCubeMapSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front));
      assert(LoadCubeMapSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back));
      assert(LoadCubeMapSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top));
      assert(LoadCubeMapSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom));
      assert(LoadCubeMapSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left));
      assert(LoadCubeMapSide(GL_TEXTURE_CUBE_MAP_POSITIVE_X, right));
      // format cube map texture
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  bool Texture::LoadCubeMapSide(GLenum side_target, const char * file_name)
  {
      int x, y;
      unsigned char* image_data = LoadTexImage(filename_, &x, &y);
      glTexImage2D(side_target, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
      stbi_image_free(image_data);
      return true;
  }

  Texture::Texture()
  {
   
  }

  Texture::Texture(char* filename)
  {
    Load(filename);
  }

  Texture::~Texture()
  {
    GLCall(glDeleteTextures(1, &texture_id_));
  }
} //namespace scene
