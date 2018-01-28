#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "glm/glm.hpp"

namespace scene {
    struct Mesh {
        render::VertexArray va;
        render::IndexBuffer vb;
    };
    class Object {
    private:
        Mesh mesh_;
        render::Shader* shader_;
        //Some translations
        Object* parent; //Heirarchy
    public:
        Object();
        ~Object();
    };
} //namespace scene