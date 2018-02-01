#include "Shader.h"
#include "ShaderLoader.h"
#include "ErrorHandling.h"
#include <fstream>
#include <string.h>

namespace render {
    Shader::Shader()
        :renderer_id_(0) {}
    Shader::Shader(const std::string type, const std::string shaders)
        : name_(type)
    {
        std::ifstream shader_names(shaders);
        std::string name;
        core::ShaderLoader shaderloader;
        char* filenames[2];
        bool done = false;
        while (!done) {
            if (!getline(shader_names, name)) {
                fprintf(stderr, "Did not find shaders to use for %s\n", type.c_str());
                exit(-1);
            }
            if (name == name_) {
                getline(shader_names, name);
                char * cstr = new char[name.length() + 1];
                if (strcpy_s(cstr, name.length() + 1, name.c_str())) {
                    fprintf(stderr, "Error copying string %s", name.c_str());
                    exit(-1);
                }
                filenames[0] = cstr;
                getline(shader_names, name);
                cstr = new char[name.length() + 1];
                if (strcpy_s(cstr, name.length() + 1, name.c_str())) {
                    fprintf(stderr, "Error copying string %s", name.c_str());
                    exit(-1);
                }
                filenames[1] = cstr;
                done = true;
            }
        }
        renderer_id_ = shaderloader.CreateProgram(filenames);
        filenames_ = new char*[2];
        filenames_[0] = filenames[0];
        filenames_[1] = filenames[1];
        shader_names.close();
    }

    Shader::Shader(char ** filenames) 
        :name_(filenames[0]), filenames_(filenames)
    {
        core::ShaderLoader shaderloader;
        renderer_id_ = shaderloader.CreateProgram(filenames);
    }
    //TODO I am having some scoping problems that I would like to fix later
    Shader::~Shader()
    {
        GLCall(glDeleteProgram(renderer_id_));
    delete[](filenames_);
    }

    void Shader::Bind() const
    {
        GLCall(glUseProgram(renderer_id_));
    }

    void Shader::UnBind() const
    {
        GLCall(glUseProgram(0));
    }

    void Shader::Reload()
    {
        GLCall(glDeleteProgram(renderer_id_));
        uniform_location_cache_.clear();
        core::ShaderLoader shaderloader;
        renderer_id_ = shaderloader.CreateProgram(filenames_);
    }

    void Shader::SetUniform3f(const std::string name, glm::vec3 value)
    {
        GLCall(glUniform3fv(GetUniformLocation(name), 1, &value[0]));
    }

  void Shader::SetUniform4fv(const std::string name, glm::mat4 value)
  {
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]));
  }

  void Shader::SetUniform1f(const std::string name, float value)
  {
    GLCall(glUniform1f(GetUniformLocation(name), value));
  }

  GLuint Shader::GetUniformLocation(const std::string name)
  {
      if (uniform_location_cache_.find(name) != uniform_location_cache_.end())
          return uniform_location_cache_[name];
      GLCall(GLint location = glGetUniformLocation(renderer_id_, name.c_str()));
      if (location < 0) {
          fprintf(stderr, "Got negative Uniform location for %s in %s\n", name.c_str(), name_.c_str());
      }
      uniform_location_cache_[name] = location;
      return location;
  }
  void CommonShader::SetUniforms(glm::mat4 view, glm::mat4 proj, glm::mat4 model, glm::vec3 colour)
  {
    SetUniform4fv("view", view);
    SetUniform4fv("proj", proj);
    SetUniform4fv("model", model);
    SetUniform4fv("mv_it", glm::transpose(glm::inverse(view * model)));
    SetUniform3f("colour", colour);
  }
} //namespace render