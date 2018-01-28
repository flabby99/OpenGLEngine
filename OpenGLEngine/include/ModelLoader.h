#pragma once
#include <string>
#include <vector>
#include "glew/glew.h"
#include "VertexArray.h"
//TODO later decouple vas and ibs
#include "IndexBuffer.h"

//Forward declare structs
struct aiScene;
struct aiMesh;

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
	std::pair<render::VertexArray, render::IndexBuffer> GetData(int index);
private:
	//Data that is later retrieved in main
	std::vector<std::pair<render::VertexArray, render::IndexBuffer>> output;
	unsigned int NumMeshes = 0;

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
};