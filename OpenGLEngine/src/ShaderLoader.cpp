#include "ShaderLoader.h"
#include <fstream>

using namespace std;
namespace core {
	ShaderLoader::ShaderLoader() {}
	ShaderLoader::~ShaderLoader() {}

	char* ShaderLoader::ReadShader(const char * filename)
	{
		ifstream file(filename, ifstream::in | ifstream::binary | ifstream::ate); //ate indicates start at eof
		if (!file) {
			fprintf(stderr, "Error: Could not open %s for reading", filename);
			exit(-1);
		}
		streampos size = file.tellg();
		char *shadercode;
		shadercode = new char[size + (streampos)1];
		file.seekg(0, file.beg);
		file.read(shadercode, size);
		file.close();
		shadercode[size] = 0;
		return shadercode;
	}

	GLuint ShaderLoader::CreateShader(GLenum shadertype, const char* shadercode)
	{
		GLint compile_result = 0;
		GLuint shader = glCreateShader(shadertype);
		if (!shader) {
			fprintf(stderr, "Error: Could not create shader type %d\n", shadertype);
			exit(-1);
		}
		glShaderSource(shader, 1, (const GLchar**)&shadercode, NULL);
		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);
		//Taken from Rachel's code, logs the error if there is one from above:
		if (compile_result == GL_FALSE) {
			GLchar infolog[1024];
			glGetShaderInfoLog(shader, 1024, NULL, infolog);
			fprintf(stderr, "Error compiling shader type %d: '%s'\n", shadertype, infolog);
			exit(1);
		}
		return shader;
	}

	//Currently only deals with a vertex shader and a fragment shader
	GLuint ShaderLoader::CreateProgram(char ** filenames)
	{
		GLint success = 0;
		GLchar errorlog[1024];
		//Begin by reading shader files and then creating a shader for each
		char* vertexshader_code = ReadShader(filenames[0]);
		char* fragmentshader_code = ReadShader(filenames[1]);

		GLuint vertexshader = CreateShader(GL_VERTEX_SHADER, vertexshader_code);
		GLuint fragmentshader = CreateShader(GL_FRAGMENT_SHADER, fragmentshader_code);

		GLuint program = glCreateProgram();
		glAttachShader(program, vertexshader);
		glAttachShader(program, fragmentshader);
		glLinkProgram(program);
		//Clean up shaders
		glDetachShader(program, vertexshader);
		glDetachShader(program, fragmentshader);
		glDeleteShader(vertexshader);
		glDeleteShader(fragmentshader);
		//The following is from Rachel's code again to error check.
		//check for program related errors using glGetProgramiv
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program, sizeof(errorlog), NULL, errorlog);
			fprintf(stderr, "Error linking shader program: '%s'\n", errorlog);
			exit(-1);
		}

		//Program has been successfully linked but needs to be validated
		//Only do this during development as it is expensive
		glValidateProgram(program);
		//check for program related errors using glGetProgramiv
		glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program, sizeof(errorlog), NULL, errorlog);
			fprintf(stderr, "Invalid shader program: '%s'\n", errorlog);
			exit(-1);
		}

		return program;
    }
} //namespace core
