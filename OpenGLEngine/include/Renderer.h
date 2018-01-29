#pragma once
#include "Object.h"
//REFERENCE inspired by https://www.youtube.com/watch?v=jjaTTRFXRAk
//The Cherno Project
namespace render {
	class Renderer {
     //This should bind the vao and ib in object, set up any tranforms, set uniforms, bind the shader, and draw triangles
		void Draw(scene::Object object, const Shader* shader);
    //Clears the depth and colour buffers
    void Clear() const; 
	};
} //namespace render