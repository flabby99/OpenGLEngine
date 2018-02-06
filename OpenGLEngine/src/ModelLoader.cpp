#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "ModelLoader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace core {
  SceneInfo::SceneInfo()
  {
  }

  SceneInfo::~SceneInfo()
  {
  }

  bool SceneInfo::LoadModelFromFile(const std::string& Filename)
  {
    // Create an instance of the Importer class
    Assimp::Importer importer;
    //There are present options that you can pass to readfile like aiProcessPreset_TargetRealtime_Quality
    const aiScene* scene = importer.ReadFile(Filename,
      aiProcess_CalcTangentSpace |
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_SortByPType |
      aiProcess_GenSmoothNormals |
      aiProcess_JoinIdenticalVertices);
    // If the import failed, report it
    if (!scene)
    {
      fprintf(stderr, importer.GetErrorString());
      return false;
    }
    // Now we can access the file's contents.
    fprintf(stderr, "Read %d meshes from %s\n", scene->mNumMeshes, Filename.c_str());
    return InitFromScene(scene, Filename);
  }

  void SceneInfo::CopyVectors()
  {
    MeshInfo meshinfo;
    meshinfo.positions = positions;
    meshinfo.normals = normals;
    meshinfo.textures = textures;
    meshinfo.indices = indices;
    meshes.push_back(meshinfo);
  }

  void SceneInfo::ClearVectors()
  {
    positions.clear();
    normals.clear();
    textures.clear();
    indices.clear();
  }

  bool SceneInfo::InitFromScene(const aiScene* Scene, const std::string& Filename) {
    NumMeshes = Scene->mNumMeshes;
    for (int i = 0; i < NumMeshes; ++i) {
      const aiMesh* aiMesh = Scene->mMeshes[i];
      InitMesh(aiMesh);
      CopyVectors();
      ClearVectors();
    }
    white = new scene::Texture("res/Models/textures/white.jpg");
    if(Scene->HasMaterials())
        return InitMaterials(Scene, Filename);
    has_materials = false;
    return true;
  }

  void SceneInfo::InitMesh(const aiMesh* aiMesh) {
    const aiVector3D zero(0.0f, 0.0f, 0.0f);
    //Extract the vertex positions, normals and texture co-ordinates
    for (int i = 0; i < aiMesh->mNumVertices; ++i) {
      const aiVector3D* pos = &(aiMesh->mVertices[i]);
      const aiVector3D* normal = &(aiMesh->mNormals[i]);
      const aiVector3D* tex_coord = aiMesh->HasTextureCoords(0) ? &(aiMesh->mTextureCoords[0][i]) : &zero;
      positions.push_back(pos->x);
      positions.push_back(pos->y);
      positions.push_back(pos->z);
      normals.push_back(normal->x);
      normals.push_back(normal->y);
      normals.push_back(normal->z);
      textures.push_back(tex_coord->x);
      textures.push_back(tex_coord->y);
    }

    for (int i = 0; i < aiMesh->mNumFaces; ++i) {
      const aiFace& face = aiMesh->mFaces[i];
      //Ensure that the face is indeed a triangle.
      assert(face.mNumIndices == 3);
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
    }
    material_indices.push_back(aiMesh->mMaterialIndex);
  }

  //Placeholder code to deal with materials and textures
  bool SceneInfo::InitMaterials(const aiScene * Scene, const std::string & Filename)
  {
    for (int i = 0; i < Scene->mNumMaterials; ++i) {
        aiMaterial* Material = Scene->mMaterials[i];
        bool skip = false;
        std::string dir = "res/Models/";
        if (Material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;
            if (Material->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string full_path = dir + Path.data;
                //If the texture is not loaded, then load it
                for (auto& texture_ptr : loaded_textures) {
                    if (strcmp(texture_ptr->GetFileName(), full_path.c_str()) == 0) {
                        object_textures.push_back(texture_ptr);
                        skip = true;
                    }
                }
                if (!skip) {
                    char * cstr = new char[full_path.length() + 1];
                    if (strcpy_s(cstr, full_path.length() + 1, full_path.c_str())) {
                        fprintf(stderr, "Error copying string %s", full_path.c_str());
                        exit(-1);
                    }
                    scene::Texture *texture = new scene::Texture(cstr);
                    loaded_textures.push_back(texture);
                    object_textures.push_back(texture);
                }
            }
            else {
                fprintf(stderr, "ERROR: Could not find diffuse texture at %s", Path.data);
                exit(-1);
            }
        }
        else object_textures.push_back(white);
    }
    return true;
  }

  //This lends itself nicely to being able to work with heirarchical transforms
  void SceneInfo::InitBuffersAndArrays() {
    for (int i = 0; i < NumMeshes; ++i)
    {
      render::VertexBuffer points_vbo(meshes[i].positions.data(), meshes[i].positions.size() * sizeof(GLfloat));
      render::VertexBuffer normals_vbo(meshes[i].normals.data(), meshes[i].normals.size() * sizeof(GLfloat));
      render::VertexArray va;
      va.Addbuffer_3f(points_vbo, 0);
      va.Addbuffer_3f(normals_vbo, 1);
      if (has_materials) {
          render::VertexBuffer textures_vbo(meshes[i].textures.data(), meshes[i].textures.size() * sizeof(GLfloat));
          va.Addbuffer_2f(textures_vbo, 2);
      }
      
      scene::Object* object = new scene::Object(va, render::IndexBuffer(meshes[i].indices.data(), meshes[i].indices.size()));
      //TODO this will not be the same size as the number of meshes
      if (has_materials) {
          object->SetDiffuseTexture(object_textures[material_indices[i]]);
      }
      else object->SetDiffuseTexture(white);
      output.push_back(object);
    }
  }

  int SceneInfo::GetNumMeshes()
  {
    return NumMeshes;
  }

  int SceneInfo::GetNumIndices(int index)
  {
    return meshes[index].indices.size();
  }

  scene::Object* SceneInfo::GetObject_(int index)
  {
    return output[index];
  }
} //namespace core

