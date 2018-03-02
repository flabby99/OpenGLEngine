#pragma once
#include "glew\glew.h"
namespace render {
    //TODO could consider expanding this to work with subdata!
    //glBufferData(GL_ARRAY_BUFFER, numVertices * 7 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
    //// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
    //glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * 3 * sizeof(GLfloat), vertices);
    //glBufferSubData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), numVertices * 4 * sizeof(GLfloat), colors);
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