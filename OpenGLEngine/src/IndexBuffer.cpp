#include "IndexBuffer.h"
#include "ErrorHandling.h"

namespace render {
    IndexBuffer::IndexBuffer() {}
    IndexBuffer::IndexBuffer(const GLuint* data, unsigned int count) 
        : count_(count)
    {
        GLCall(glGenBuffers(1, &renderer_id_));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_));
        GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data, GL_STATIC_DRAW));
    }

    IndexBuffer::~IndexBuffer()
    {
        //GLCall(glDeleteBuffers(1, &renderer_id_));
    }

    void IndexBuffer::Bind() const
    {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_id_));
    }

    void IndexBuffer::Unbind() const
    {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }
} //namespace render