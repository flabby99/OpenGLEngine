#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

namespace render {
    class VertexArray
    {
    public:
        VertexArray();
        ~VertexArray();
        void Bind() const;
        void Unbind() const;
        void Addbuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
    private:
        GLuint renderer_id_;
    };

} //namespace render