#pragma once
#include "glm/glm.hpp"
#include "glew\glew.h"
#include <string>
#include <unordered_map>

namespace render {
    class Shader
    {
    public:
        Shader(const std::string type, const std::string shaders);
        Shader(char** filenames);
        ~Shader();
        void Bind() const;
        void UnBind() const;

        //Set Uniforms
        //Could Template this maybe?
        void SetUniform3f(const std::string name, glm::vec3 value);
    private:
        GLuint renderer_id_;
        const std::string name_;
        std::unordered_map<std::string, GLint> uniform_location_cache_;
        GLuint GetUniformLocation(const std::string name);
    };


} //namespace render