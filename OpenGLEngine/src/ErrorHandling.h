#pragma once
#include <string>
#include "glm\glm.hpp"
#include <iostream>
//Used for error handling in GL
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) error_handling::GLErrors::GLClearErrors();\
    x;\
    ASSERT(error_handling::GLErrors::GLLogCall(#x, __FILE__, __LINE__))

namespace error_handling {
    //The intention would be to call clear before drawing and check after drawing
    class GLErrors {
    public:
        static void GLClearErrors();
        static bool GLLogCall(const char* function, const char* file, int line);
    };
    class Log {
    public:
      inline static void LogToCout(glm::vec3 value) {
        std::cout << "Vector is: " << value.x << " " << value.y << " " << value.z << std::endl;
      }
    };
} //namespace error_handling