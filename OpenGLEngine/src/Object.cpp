#include "Object.h"
#include "ModelLoader.h"
#include "glm/gtc/matrix_transform.hpp"

namespace scene {
  Object::Object()
  {
    rotation_ = glm::mat4(1.0f);
    scale_ = glm::vec3(1.0f);
    translation_ = glm::vec3(0.0f);
    origin_offset_ = glm::vec3(0.0f);
    UpdateModelMatrix();
    colour_ = glm::vec3(1.0f);
    parent_ = NULL;
  }

  Object::Object(const render::VertexArray& va, const render::IndexBuffer& ib)
  {
    mesh_.va = va;
    mesh_.ib = ib;
    rotation_ = glm::mat4(1.0f);
    scale_ = glm::vec3(1.0f);
    translation_ = glm::vec3(0.0f);
    origin_offset_ = glm::vec3(0.0f);
    UpdateModelMatrix();
    colour_ = glm::vec3(1.0f);
    parent_ = NULL;
  }

  Object::~Object()
  {
  }
  void Object::UpdateModelMatrix()
  {
    model_matrix_ = glm::translate(rotation_ * glm::scale(glm::mat4(1.0f), scale_), translation_);
  }
  void Object::RotateAboutPivotPoint(float angle, glm::vec3 axis)
  {
    glm::mat4 rotation = glm::translate(glm::mat4(1.0f), origin_offset_);
    rotation = glm::rotate(rotation, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    rotation = glm::translate(rotation, -origin_offset_);
    rotation_ *= rotation;
    UpdateModelMatrix();
  }
  glm::mat4 Object::GetGlobalModelMatrix() const
  {
    glm::mat4 global = model_matrix_;
    if (parent_ != NULL) {
      global = parent_->GetGlobalModelMatrix() * model_matrix_;
    }
    return global;
  }

  void Object::AddMaterialData(const core::MaterialData& material_data) {
    texture_diffuse_ = material_data.diffuse_texture;
  }
} //namespace scene