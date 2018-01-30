#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "Texture.h"

namespace scene {
  struct Mesh {
    render::VertexArray va;
    render::IndexBuffer ib;
  };
  class Object {
  private:
    Mesh mesh_;
    glm::mat4 rotation_;
    glm::vec3 scale_;
    glm::vec3 translation_;
    glm::mat4 model_matrix_;
    glm::vec3 origin_offset_;
    glm::vec3 colour_;
    Object* parent_; //Heirarchy
    Texture* texture_;
  public:
    Object();
    Object(const render::VertexArray& va, const render::IndexBuffer& ib);
    ~Object();
    void UpdateModelMatrix();
    //Rotates the model about its pivot point using an angle and an axis
    void RotateAboutPivotPoint(float angle, glm::vec3 axis);
    inline void SetParent(Object* parent) { parent_ = parent; }
    inline void SetRotation(const glm::mat4& rotation) { rotation_ = rotation; }
    inline void SetTranslation(const glm::vec3& translation) { translation_ = translation; }
    inline void SetScale(const glm::vec3& scale) { scale_ = scale; }
    inline void SetModelMatrix(const glm::mat4& model) { model_matrix_ = model; }
    inline void SetOriginOffset(const glm::vec3& offset) { origin_offset_ = offset; }
    inline void SetColour(const glm::vec3& colour) { colour_ = colour; }
    inline void SetTexture(Texture* texture) { texture_ = texture; }
    inline void SetMesh(const render::VertexArray& va, const render::IndexBuffer& ib) {
      mesh_.va = va; mesh_.ib = ib;
    }
    inline glm::mat4 GetLocalModelMatrix() const { return model_matrix_; }
    inline Mesh GetMesh() const { return mesh_; }
    inline glm::vec3 GetColour() const { return colour_; }
    inline Object* GetParent() const { return parent_; }
    inline Texture* GetTexture() const { return texture_; }
    //Returns the model matrix relative to the object heirarchy - recursive
    glm::mat4 GetGlobalModelMatrix() const;
  };
} //namespace scene
