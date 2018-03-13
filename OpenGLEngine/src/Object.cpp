#define GLM_ENABLE_EXPERIMENTAL
#include "Object.h"
#include "ModelLoader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp" 
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"

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

  Object::Object(const std::shared_ptr<render::VertexArray>& va, const std::shared_ptr<render::IndexBuffer>& ib)
  {
    SetMesh(va, ib);
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
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), translation_);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), scale_);
    model_matrix_ = translate * rotation_ * scale;
  }
  void Object::RotateAboutPivotPoint(float angle, glm::vec3 axis)
  {
    glm::mat4 rotation = glm::translate(glm::mat4(1.0f), origin_offset_);
    rotation = glm::rotate(rotation, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    rotation = glm::translate(rotation, -origin_offset_);
    rotation_ *= rotation;
    UpdateModelMatrix();
  }
  void Object::SetQuatRotate(const glm::quat& rotation)
  {
    glm::mat4 m_rotation = glm::translate(glm::mat4(1.0f), origin_offset_);
    m_rotation = glm::toMat4(rotation);
    m_rotation = glm::translate(m_rotation, -origin_offset_);
    rotation_ = m_rotation;
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