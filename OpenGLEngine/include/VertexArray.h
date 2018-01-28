#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "glew\glew.h"

namespace render {
    class VertexArray
    {
    public:
        VertexArray();
        ~VertexArray();
        void Bind() const;
        void Unbind() const;
        void Addbuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
        void Addbuffer_3f(const VertexBuffer& vb, const GLuint index);
    private:
        GLuint renderer_id_;
    };

} //namespace render