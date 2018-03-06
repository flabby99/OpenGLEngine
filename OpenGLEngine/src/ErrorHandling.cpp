#include "ErrorHandling.h"
#include "glew/glew.h"

namespace error_handling {
    void GLErrors::GLClearErrors()
    {
        while (glGetError() != GL_NO_ERROR);
    }
    bool GLErrors::GLLogCall(const char* function, const char* file, int line)
    {
        if (GLenum error = glGetError()) {
            std::cout << "OpenGL error: " << error << " " << function << 
                " " << file << " " << line << std::endl;
            return false;
        }
        return true;
    }
} //namespace error_handling
