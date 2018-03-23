#pragma once
#include "Object.h"
#include "glm\glm.hpp"
#include "shader.h"
#include "FrameBuffer.h"

//REFERENCE inspired by https://www.youtube.com/watch?v=jjaTTRFXRAk
//The Cherno Project
//TODO add support for drawing using framebuffers, see OpenGL tutorial 15
namespace render {
  class VertexGrid;
  class Renderer {
    Shader* shader_;
    glm::mat4 view_;
  public:
    inline Renderer(Shader* shader, glm::mat4 view) :shader_(shader), view_(view) {}
    //Also updates the modelview inverse transpose
    static void Draw(scene::Object object, Shader* shader, glm::mat4 view);
    //Draws an object using the current shader and updates no uniforms
    static void Draw(scene::Object object);
    //Draws a grid of points
    static void DrawPoints(VertexGrid* vertex_grid);
    //Draws an object using the view and shader in Renderer
    void Render(scene::Object object);
    //Clears the depth and colour buffers
    static void Clear(); 
    inline static void SetScreenAsRenderTarget() {
      GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
  };
} //namespace render