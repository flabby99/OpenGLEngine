#pragma once
#include "Object.h"
//REFERENCE inspired by https://www.youtube.com/watch?v=jjaTTRFXRAk
//The Cherno Project
namespace render {
  class Renderer {
  public:
	//Also updates the modelview inverse transpose
	static void Draw(scene::Object object, Shader* shader, glm::mat4 view);
  //Draws an object using the current shader and updates no uniforms
  static void Draw(scene::Object object);
  //Clears the depth and colour buffers
  static void Clear(); 
  };
} //namespace render