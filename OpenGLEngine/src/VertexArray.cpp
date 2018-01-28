#include "VertexArray.h"
#include "ErrorHandling.h"

namespace render {

    VertexArray::VertexArray()
    {
        GLCall(glGenVertexArrays(1, &renderer_id_));
    }

    VertexArray::~VertexArray()
    {
        //GLCall(glDeleteVertexArrays(1, &renderer_id_));
    }

    void VertexArray::Bind() const {
        GLCall(glBindVertexArray(renderer_id_));
    }

    void VertexArray::Unbind() const {
        GLCall(glBindVertexArray(0));
    }

    void VertexArray::Addbuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
    {
        Bind();
        vb.Bind();
        const auto& elements = layout.GetElements();
        //TODO unsure of the use of offset
        unsigned int offset = 0;
        for (unsigned int i = 0; i < elements.size(); ++i) {
            const auto& element = elements[i];
            GLCall(glEnableVertexAttribArray(i));
            GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalised, 
                layout.GetStride(), (const GLvoid*)offset));
            offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
        }
    }

    void VertexArray::Addbuffer_3f(const VertexBuffer& vb, const GLuint index) {
        Bind();
        vb.Bind();
        GLCall(glEnableVertexAttribArray(index));
        GLCall(glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, NULL))
    }
} //namespace render