#pragma once
#include "glew\glew.h"
namespace render {
    class IndexBuffer
    {
    public:
        IndexBuffer(const GLuint* data, unsigned int count);
        ~IndexBuffer();
        void Bind() const;
        void Unbind() const;

        inline unsigned int GetCount() const { return count_; }

    private:
        GLuint renderer_id_;
        GLuint count_;
    };
} //namespace render