#pragma once
#include <vector>
#include <glew/glew.h>

namespace render {
    class VertexBufferElement
    {
    public:
        GLuint type;
        unsigned int count;
        unsigned char normalised;

        static unsigned int GetSizeOfType(GLuint type)
        {
            switch (type)
            {
            case GL_FLOAT: return 4;
            case GL_UNSIGNED_INT: return 4;
            case GL_UNSIGNED_BYTE: return 1;
            }
            fprintf(stderr, "Could not find size of type %d", type);
            return 0;
        }
    };

    class VertexBufferLayout
    {
    private:
        std::vector<VertexBufferElement> elements_;
        unsigned int stride_;
    public:
        VertexBufferLayout()
            :stride_(0) {}

        template<typename T>
        void Push(unsigned int count)
        {
            static_assert(false);
        }

        template<>
        void Push<float>(unsigned int count)
        {
            elements_.push_back({ GL_FLOAT, count, GL_FALSE });
            stride_ += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
        }

        template<>
        void Push<unsigned int>(unsigned int count)
        {
            elements_.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
            stride_ += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
        }
        template<>
        void Push<unsigned char>(unsigned int count)
        {
            elements_.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
            stride_ += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
        }

        inline const std::vector<VertexBufferElement>& GetElements() const { return elements_; };
        inline unsigned int GetStride() const { return stride_; };
    };
} //namespace render