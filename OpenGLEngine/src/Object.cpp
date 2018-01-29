#include "..\include\Object.h"
#include "glm/gtc/matrix_transform.hpp"

namespace scene{
	Object::Object()
	{
		rotation_ = glm::mat4(1.0f);
		scale_ = glm::vec3(1.0f);
		translation_ = glm::vec3(0.0f);
		UpdateModelMatrix();
		parent_ = NULL;
	}

	Object::Object(const render::VertexArray& va, const render::IndexBuffer& ib)
	{
		mesh_.va = va;
		mesh_.ib = ib;
		rotation_ = glm::mat4(1.0f);
		scale_ = glm::vec3(1.0f);
		translation_ = glm::vec3(0.0f);
		UpdateModelMatrix();
		parent_ = NULL;
	}

	Object::~Object()
	{
	}
	void Object::UpdateModelMatrix()
	{
		model_matrix_ = glm::translate(rotation_ * glm::scale(glm::mat4(1.0f), scale_), translation_);
	}
	glm::mat4 Object::GetGlobalModelMatrix() const
	{
		glm::mat4 global = model_matrix_;
		if (parent_ != NULL) {
			global = parent_->GetGlobalModelMatrix() * global;
		}
		return global;
	}
} //namespace scene
