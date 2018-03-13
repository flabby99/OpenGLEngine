#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "ModelLoader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "util.h"
#include "Object.h"
#include <iostream>

namespace core {
  SceneInfo::SceneInfo(std::string base_dir, std::string name, std::shared_ptr<scene::Texture> white):
    base_dir_(base_dir), white_(white)
  {
    vertex_buffer_layout_ = render::VertexBufferLayout();
    //Three position co ords
    vertex_buffer_layout_.Push<float>(3);
    //Three normal co ords
    vertex_buffer_layout_.Push<float>(3);
    //Two texture co ords
    vertex_buffer_layout_.Push<float>(2);
    //Three tangent co ords
    vertex_buffer_layout_.Push<float>(3);

    std::string filename = base_dir + name;
    if (!LoadModelFromFile(filename)) {
      fprintf(stderr, "ERROR: Could not load %s", filename.c_str());
      exit(-1);
    }
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

  bool SceneInfo::InitFromScene(const aiScene* Scene, const std::string& Filename) {
    if(!white_) white_ = std::make_shared<scene::Texture>("res/Models/textures/white.jpg");
    has_materials_ = Scene->HasMaterials();
    for (unsigned int i = 0; i < Scene->mNumMeshes; ++i) {
      const aiMesh* aiMesh = Scene->mMeshes[i];
      if (!aiMesh->HasNormals()) fprintf(stderr, "WARNING: Model %d in %s has no normals\n", i, Filename.c_str());
      if (!aiMesh->HasTextureCoords(0)) fprintf(stderr, "WARNING: Model %d in %s has no tex coords\n", i, Filename.c_str());
      std::shared_ptr<scene::Object> object = InitMesh(aiMesh);
      MaterialData material_data = LoadMaterial(Scene, aiMesh->mMaterialIndex);
      object->AddMaterialData(material_data);
      objects_.push_back(object);
    }
    return true;
  }

  std::shared_ptr<scene::Object> SceneInfo::InitMesh(const aiMesh* aiMesh) {
    const aiVector3D zero(0.0f, 0.0f, 0.0f);
    std::unique_ptr<VertexData[]> vertices = std::make_unique<VertexData[]>(aiMesh->mNumVertices);
    //Extract the vertex positions, normals and texture co-ordinates
    for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {

      const aiVector3D* pos = &(aiMesh->mVertices[i]);
      const aiVector3D* normal = &(aiMesh->mNormals[i]);
      const aiVector3D* tex_coord = aiMesh->HasTextureCoords(0) ? &(aiMesh->mTextureCoords[0][i]) : &zero;
      const aiVector3D* tangent = aiMesh->HasTextureCoords(0) ? &aiMesh->mTangents[i] : &zero;
      vertices[i].pos_x_ = pos->x;
      vertices[i].pos_y_ = pos->y;
      vertices[i].pos_z_ = pos->z;
      
      vertices[i].normal_x_ = normal->x;
      vertices[i].normal_y_ = normal->y;
      vertices[i].normal_z_ = normal->z;
      
      vertices[i].tex_coord_x_ = tex_coord->x;
      vertices[i].tex_coord_y_ = tex_coord->y;

      vertices[i].tangent_x_ = tangent->x;
      vertices[i].tangent_y_ = tangent->y;
      vertices[i].tangent_z_ = tangent->z;
    }

    auto vb = std::make_shared<render::VertexBuffer>(vertices.get(), (unsigned int)sizeof(VertexData) * aiMesh->mNumVertices);
    auto va = std::make_shared<render::VertexArray>();
    va->Addbuffer(vb, vertex_buffer_layout_);

    std::unique_ptr<unsigned int[]> indices = std::make_unique<unsigned int[]>(3 * aiMesh->mNumFaces);
    for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i) {
      const aiFace& face = aiMesh->mFaces[i];
      //Ensure that the face is indeed a triangle.
      assert(face.mNumIndices == 3);
    
      indices[3 * i] = face.mIndices[0];
      indices[3 * i + 1] = face.mIndices[1];
      indices[3 * i + 2] = face.mIndices[2];
    }

    auto ib = std::make_shared<render::IndexBuffer>(indices.get(), 3 * aiMesh->mNumFaces);

    std::shared_ptr<scene::Object> object = std::make_shared<scene::Object>(va, ib);
    return object;
  }

  MaterialData SceneInfo::LoadMaterial(const aiScene* Scene, const unsigned int& material_index) {
    aiMaterial* Material = Scene->mMaterials[material_index];
    MaterialData material_data;
    //NOTE could refactor this out into getting a texture
    if (Material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
      aiString Path;
      if (Material->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
        bool skip = false;
        std::string full_path = Path.data;
        if(Path.data[1] != ':') full_path = base_dir_ + Path.data;
        //If the texture is not loaded, then load it
        for (auto& texture_ptr : loaded_textures) {
          if (strcmp(texture_ptr->GetFileName(), full_path.c_str()) == 0) {
            material_data.diffuse_texture = texture_ptr;
            skip = true;
            break;
          }
        }
        if (!skip) {
          std::shared_ptr<scene::Texture> texture = std::make_shared<scene::Texture>(full_path.c_str());
          loaded_textures.push_back(texture);
          material_data.diffuse_texture = texture;
          std::cout << "got here!" << std::endl;
        }
      }
      else {
        fprintf(stderr, "ERROR: Could not find diffuse texture at %s", Path.data);
        exit(-1);
      }
    }
    else material_data.diffuse_texture = white_;
    return material_data;
  }

  std::shared_ptr<scene::Object> SceneInfo::GetObject_(int index)
  {
    return (objects_[index]);
  }
} //namespace core

