#pragma once
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "glew\glew.h"
#include <memory>
#include <list>

namespace render {
    class VertexArray
    {
    public:
        VertexArray();
        ~VertexArray();
        void Bind() const;
        void Unbind() const;
        void Addbuffer(const std::shared_ptr<VertexBuffer>& vb, const VertexBufferLayout& layout);
        void Addbuffer_3f(const std::shared_ptr<VertexBuffer>& vb, const GLuint index);
        void Addbuffer_2f(const std::shared_ptr<VertexBuffer>& vb, const GLuint index);
    private:
        GLuint renderer_id_;
        std::list<std::shared_ptr<VertexBuffer>> attached_vbs;
    };

} //namespace render