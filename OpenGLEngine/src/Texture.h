#pragma once
#include "glew/glew.h"
#include <string>

//REFERENCE: Largely inspried by Dr Anton Geraldans book - Anton's OpenGL 4 Tutorials
namespace scene {
  class Texture {
    GLuint texture_id_;
    char* filename_;
    GLenum type_;
    GLenum slot_ = GL_TEXTURE0;

  public:
    void Bind();
    void Load(char* filename);
    void LoadNoMip(char* filename);
    //Loads the tex image in filename, storing the dimensions in x and y
    unsigned char* LoadTexImage(const char* filename, int* x, int* y, bool flip);
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
    inline void SetSlot(GLenum slot) { slot_ = slot; }
  };
} //namespace scene