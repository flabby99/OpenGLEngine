#pragma once
#include "glew\glew.h"
namespace render {
    class VertexBuffer
    {
    public:
        VertexBuffer(const void* data, unsigned int size);
        ~VertexBuffer();
        void Bind() const;
        void Unbind() const;

    private:
        GLuint renderer_id_;
    };
} //namespace render