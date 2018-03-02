#pragma once
#include <string>
#include <vector>
#include "glew/glew.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "VertexBufferLayout.h"

//Forward declare structs
struct aiScene;
struct aiMesh;
namespace scene {
  class Object;
}

//The basic idea is that we should read everything into the buffers as we go to avoid storing
namespace core {
  struct VertexData {
    GLfloat pos_x_, pos_y_, pos_z_;
    GLfloat normal_x_, normal_y_, normal_z_;
    GLfloat tex_coord_x_, tex_coord_y_;
    GLfloat tangent_x_, tangent_y_, tangent_z_;
  };

  //Could make this a class, and overwrite the class based on the model loaded
  struct MaterialData {
    std::shared_ptr<scene::Texture> diffuse_texture;
  };

  //Handles loading a model from a file
  //After loading stores a vector of Objects
  class SceneInfo {
  public:
    //Delegating constructors here
    //Base - intiliases base directory for model assets and the white texture
    SceneInfo(std::string base_dir, std::string name, std::shared_ptr<scene::Texture> white);
    inline SceneInfo(std::string name) 
      : SceneInfo("res/Models/", name, nullptr) {}
    //Use the default base dir of res models
    inline SceneInfo(std::string name, std::shared_ptr<scene::Texture> white) 
      : SceneInfo("res/Models/", name, white) {}
    inline SceneInfo() {}
    ~SceneInfo();
    bool LoadModelFromFile(const std::string& Filename);
    std::shared_ptr<scene::Object> GetObject_(int index);
    inline const unsigned int GetNumMeshes() {
      return (unsigned int) objects_.size();
    }
  private:
    std::string base_dir_;
    //Data that is later retrieved in main
    render::VertexBufferLayout vertex_buffer_layout_;
    std::vector<std::shared_ptr<scene::Object>> objects_;
    std::vector<std::shared_ptr<scene::Texture>> loaded_textures;
    bool has_materials_;
    std::shared_ptr<scene::Texture> white_;

    //Private functions
    bool InitFromScene(const aiScene* Scene, const std::string& Filename);
    std::shared_ptr<scene::Object> InitMesh(const aiMesh* aiMesh);
    //NOTE could consider Loading all the materials at start, storing these, then just getting them
    MaterialData LoadMaterial(const aiScene* Scene, const unsigned int& material_index);
  };
} //namespace core