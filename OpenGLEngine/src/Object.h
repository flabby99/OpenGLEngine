#pragma once
#include <memory>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "glm/glm.hpp"
#include "Texture.h"
#include <memory>

namespace core {
  struct MaterialData;
}

namespace scene {
  struct Mesh {
    std::shared_ptr<render::VertexArray> va;
    std::shared_ptr<render::IndexBuffer> ib;
  };
  class Object {
  private:
    Mesh mesh_;
    unsigned int num_indices_;
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
    Object(const std::shared_ptr<render::VertexArray>& va, const std::shared_ptr<render::IndexBuffer>& ib);
    ~Object();
    void UpdateModelMatrix();
    //Rotates the model about its pivot point using an angle and an axis
    void AddMaterialData(const core::MaterialData& material_data);
    void RotateAboutPivotPoint(float angle, glm::vec3 axis);
    void SetQuatRotate(const glm::quat& rotation);
    inline void SetNumIndices(const unsigned int& value) { num_indices_ = value; }
    inline void SetParent(const std::shared_ptr<Object>& parent) { parent_ = parent; }
    inline void SetRotation(const glm::mat4& rotation) { rotation_ = rotation; }
    inline void SetTranslation(const glm::vec3& translation) { translation_ = translation; }
    inline void SetScale(const glm::vec3& scale) { scale_ = scale; }
    inline void SetModelMatrix(const glm::mat4& model) { model_matrix_ = model; }
    inline void SetOriginOffset(const glm::vec3& offset) { origin_offset_ = offset; }
    inline void SetColour(const glm::vec3& colour) { colour_ = colour; }
    inline void SetDiffuseTexture(const std::shared_ptr<Texture>& texture) { texture_diffuse_ = texture; }
    inline void SetBumpTexture(const std::shared_ptr<Texture>& texture) { texture_bump_ = texture; }
    inline void SetNormalTexture(const std::shared_ptr<Texture>& texture) { texture_normal_ = texture; }
    inline void SetMesh(const std::shared_ptr<render::VertexArray>& va, const std::shared_ptr<render::IndexBuffer>& ib) {
      mesh_.va = va; mesh_.ib = ib;
    }
    inline glm::mat4 GetLocalModelMatrix() const { return model_matrix_; }
    inline Mesh GetMesh() const { return mesh_; }
    inline unsigned int GetNumIndices() const { return num_indices_; }
    inline glm::vec3 GetColour() const { return colour_; }
    inline std::shared_ptr<Object> GetParent() const { return parent_; }
    inline std::shared_ptr<Texture> GetDiffuseTexture() const { return texture_diffuse_; }
    inline std::shared_ptr<Texture> GetBumpTexture() const { return texture_bump_; }
    inline std::shared_ptr<Texture> GetNormalTexture() const { return texture_normal_; }
    inline glm::vec3 GetScale() const { return scale_; }
    //Returns the model matrix relative to the object heirarchy - recursive
    glm::mat4 GetGlobalModelMatrix() const;
  };
} //namespace scene
