#pragma once
#include <string>

namespace error_handling {
    //The intention would be to call clear before drawing and check after drawing
    class GLErrors {
    public:
        static void GLClearErrors();
        static bool GLLogCall(const char* function, const char* file, int line);
    };
} //namespace error_handling