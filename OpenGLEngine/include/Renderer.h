#pragma once
#include "Object.h"
//REFERENCE inspired by https://www.youtube.com/watch?v=jjaTTRFXRAk
//The Cherno Project
namespace render {
  class Renderer {
  public:
     //This should bind the vao and ib in object, set up any tranforms, set uniforms, bind the shader, and draw triangles
    static void Draw(scene::Object object, Shader* shader);
	//Also updates the modelview inverse transpose
	static void Draw(scene::Object object, Shader* shader, glm::mat4 view);
    //Clears the depth and colour buffers
    static void Clear(); 
  };
} //namespace render