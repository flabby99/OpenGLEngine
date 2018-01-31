#pragma once
#include "glew/glew.h"
#include <string>

//REFERENCE: Largely inspried by Dr Anton Geraldans book - Anton's OpenGL 4 Tutorials
namespace scene {
  class Texture {
    GLuint texture_id_;
    char* filename_;
    GLenum type_;

  public:
    void Bind();
    void Load(char* filename);
    //Loads the tex image in filename, storing the dimensions in x and y
    unsigned char* LoadTexImage(const char* filename, int* x, int* y);
    void CreateCubeMap(const char* front,
        const char* back,
        const char* top,
        const char* bottom,
        const char* left,
        const char* right);
    bool LoadCubeMapSide(GLenum side_target, const char* filename);
    Texture();
    Texture(char* filename);
    ~Texture();
    inline char* GetFileName() const { return filename_; }
  };
} //namespace scene