#pragma once
#include "glm/glm.hpp"
#include "glew\glew.h"
#include <string>
#include <unordered_map>

namespace render {
    class Shader
    {
    public:
        // @param type - a string indicating which type of shader you are loading
        // @param shaders - the name of the file containing the shaders
        Shader();
        Shader(const std::string type, const std::string shaders);
        Shader(char** filenames);
        ~Shader();
        void Bind() const;
        void UnBind() const;
        void Reload();

        //Set Uniforms
        //Could Template this maybe?
        void SetUniform3f(const std::string name, glm::vec3 value);
    private:
        GLint renderer_id_;
        std::string name_;
        char** filenames_;
        std::unordered_map<std::string, GLint> uniform_location_cache_;
        GLuint GetUniformLocation(const std::string name);
    };


} //namespace render