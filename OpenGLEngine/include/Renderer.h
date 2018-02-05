#pragma once
#include "Object.h"
#include "glm\glm.hpp"
#include "shader.h"

//REFERENCE inspired by https://www.youtube.com/watch?v=jjaTTRFXRAk
//The Cherno Project
namespace render {
  class Renderer {
    Shader* shader_;
    glm::mat4 view_;
  public:
    inline Renderer(Shader* shader, glm::mat4 view) :shader_(shader), view_(view) {}
    //Also updates the modelview inverse transpose
    static void Draw(scene::Object object, Shader* shader, glm::mat4 view);
    //Draws an object using the current shader and updates no uniforms
    static void Draw(scene::Object object);
    //Draws an object using the view and shader in Renderer
    void Render(scene::Object object);
    //Clears the depth and colour buffers
    static void Clear(); 
  };
} //namespace render