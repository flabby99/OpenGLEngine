#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "glm/glm.hpp"

namespace scene {
    struct Mesh {
        render::VertexArray va;
        render::IndexBuffer ib;
    };
    class Object {
    private:
        Mesh mesh_;
        //Some translations
		glm::mat4 rotation_;
		glm::vec3 scale_;
		glm::vec3 translation_;
		glm::mat4 model_matrix_;
        Object* parent_; //Heirarchy
    public:
        Object();
		Object(const render::VertexArray& va, const render::IndexBuffer& ib);
        ~Object();
		void UpdateModelMatrix();
		inline void SetParent(Object* parent) { parent_ = parent; }
		inline glm::mat4 GetLocalModelMatrix() const { return model_matrix_;}
		inline Mesh GetMesh() const { return mesh_; }
		//Returns the model matrix relative to the object heirarchy
		//Works recursively
		glm::mat4 GetGlobalModelMatrix() const;
    };
} //namespace scene