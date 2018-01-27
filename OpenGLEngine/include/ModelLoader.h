#pragma once
#include <string>
#include <vector>
#include "glew/glew.h"

//Forward declare structs
struct aiScene;
struct aiMesh;

//Holds the positions, normals, textures and indices of a mesh
struct MeshInfo {
	std::vector<GLfloat> positions;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> textures;
	std::vector<int> indices;
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
	std::pair<GLuint, GLuint> GetData(int index);
private:
	//Data that is later retrieved in main
	std::vector<std::pair<GLuint, GLuint>> output;
	unsigned int NumMeshes = 0;

	//Temp data, can later put this into the functions using them. 
	//They are empty after loading a model
	std::vector<GLfloat> positions;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> textures;
	std::vector<int> indices;
	std::vector<MeshInfo> meshes;
	
	//Private functions
	void CopyVectors();
	void ClearVectors();
	bool InitFromScene(const aiScene* Scene, const std::string& Filename);
	void InitMesh(const aiMesh* aiMesh);
	bool InitMaterials(const aiScene* Scene, const std::string& Filename);
	GLuint GenerateBufferObject(std::vector<GLfloat> points, GLsizeiptr points_size);
	GLuint GenerateIndexObject(std::vector<int> points, GLsizeiptr points_size);
	GLuint GenerateArrayObject(GLuint points_vbo, GLuint normals_vbo);
};