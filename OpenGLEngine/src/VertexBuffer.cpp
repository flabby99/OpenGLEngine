#include "VertexBuffer.h"
#include "ErrorHandling.h"

namespace render {
    VertexBuffer::VertexBuffer(const void* data, unsigned int size)
    {
        GLCall(glGenBuffers(1, &renderer_id_));
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, renderer_id_));
        GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
    }

    VertexBuffer::~VertexBuffer()
    {
        GLCall(glDeleteBuffers(1, &renderer_id_));
    }

    void VertexBuffer::Bind() const
    {
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, renderer_id_));
    }

    void VertexBuffer::Unbind() const
    {
        GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }
} //namespace render