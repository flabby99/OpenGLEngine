#pragma once
#include <memory>
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
    std::shared_ptr<Object> parent_ = nullptr; //Heirarchy
    std::shared_ptr<Texture> texture_diffuse_ = nullptr;
    std::shared_ptr<Texture> texture_bump_ = nullptr;
    std::shared_ptr<Texture> texture_normal_ = nullptr;
  public:
    Object();
    Object(const render::VertexArray& va, const render::IndexBuffer& ib);
    ~Object();
    void UpdateModelMatrix();
    //Rotates the model about its pivot point using an angle and an axis
    void RotateAboutPivotPoint(float angle, glm::vec3 axis);
    inline void SetParent(std::shared_ptr<Object> parent) { parent_ = parent; }
    inline void SetRotation(const glm::mat4& rotation) { rotation_ = rotation; }
    inline void SetTranslation(const glm::vec3& translation) { translation_ = translation; }
    inline void SetScale(const glm::vec3& scale) { scale_ = scale; }
    inline void SetModelMatrix(const glm::mat4& model) { model_matrix_ = model; }
    inline void SetOriginOffset(const glm::vec3& offset) { origin_offset_ = offset; }
    inline void SetColour(const glm::vec3& colour) { colour_ = colour; }
    inline void SetDiffuseTexture(std::shared_ptr<Texture> texture) { texture_diffuse_ = texture; }
    inline void SetBumpTexture(std::shared_ptr<Texture> texture) { texture_bump_ = texture; }
    inline void SetNormalTexture(std::shared_ptr<Texture> texture) { texture_normal_ = texture; }
    inline void SetMesh(const render::VertexArray& va, const render::IndexBuffer& ib) {
      mesh_.va = va; mesh_.ib = ib;
    }
    inline glm::mat4 GetLocalModelMatrix() const { return model_matrix_; }
    inline Mesh GetMesh() const { return mesh_; }
    inline glm::vec3 GetColour() const { return colour_; }
    inline std::shared_ptr<Object> GetParent() const { return parent_; }
    inline std::shared_ptr<Texture> GetDiffuseTexture() const { return texture_diffuse_; }
    inline std::shared_ptr<Texture> GetBumpTexture() const { return texture_bump_; }
    inline std::shared_ptr<Texture> GetNormalTexture() const { return texture_normal_; }
    //Returns the model matrix relative to the object heirarchy - recursive
    glm::mat4 GetGlobalModelMatrix() const;
  };
} //namespace scene
