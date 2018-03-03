#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm\glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
namespace core
{
  class Maths
  {
  public:
    Maths();
    ~Maths();
    //REFERENCE this is from OpenGL tutorials 17
    //https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.cpp
    static glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 end);
  };
} //core

