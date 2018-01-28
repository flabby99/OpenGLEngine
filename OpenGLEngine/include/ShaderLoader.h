#pragma once
#include "glew/glew.h"
#include "freeglut\freeglut.h"

namespace core {
	class ShaderLoader {
		//Read the file containing the GLSL shader code and return a string
		char* ReadShader(const char* filename);
		//Make a shader of type shadertype with code shadercode
		GLint CreateShader(GLenum shadertype, const char* shadercode);

	public:
		//Constructor and destructor:
		ShaderLoader();
		~ShaderLoader();
		//Create a program and add in all the wanted shaders to this program
		GLint CreateProgram(char** filenames);
		
	};
} //namespace core