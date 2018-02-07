#pragma once
#include <string>
#include <vector>
#include "glew/glew.h"
#include "VertexArray.h"
#include "Object.h"
#include "IndexBuffer.h"
#include "Texture.h"

//Forward declare structs
struct aiScene;
struct aiMesh;

namespace core {
  //Holds the positions, normals, textures and indices of a mesh
  struct MeshInfo {
    std::vector<GLfloat> positions;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> textures;
    std::vector<GLuint> indices;
  };

  //Handles loading a model from a file
  //After loading stores a vector of VAO index VBO pairs for each mesh, and the number of meshes, and mesh data
  class SceneInfo {
  public:
    SceneInfo();
    ~SceneInfo();
    bool LoadModelFromFile(const std::string& Filename);
    void InitBuffersAndArrays();
    int GetNumMeshes();
    int GetNumIndices(int index);
    scene::Object* GetObject_(int index);
  private:
    //Data that is later retrieved in main
    std::vector<scene::Object*> output;
    unsigned int NumMeshes = 0;
    std::vector<scene::Texture*> loaded_textures;
    std::vector<scene::Texture*> object_textures;
    std::vector<int> material_indices;
    bool has_materials = true;
    scene::Texture* white; //TODO could share this among scene infos

    //Temp data, can later put this into the functions using them. 
    //They are empty after loading a model
    std::vector<GLfloat> positions;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> textures;
    std::vector<GLuint> indices;
    std::vector<MeshInfo> meshes;

    //Private functions
    void CopyVectors();
    void ClearVectors();
    bool InitFromScene(const aiScene* Scene, const std::string& Filename);
    void InitMesh(const aiMesh* aiMesh);
    bool InitMaterials(const aiScene* Scene, const std::string& Filename);
    void ComputeTangentBasis(
      const std::vector<GLfloat> &vertices,
      const std::vector<GLfloat> &uvs,
      const std::vector<GLfloat> &normals,
      const std::vector<GLuint> &indices,
      glm::vec3* tangents,
      glm::vec3* bitangents
    );
  };
} //namespace core