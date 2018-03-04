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
    inline static glm::vec3 RotateAroundPoint(glm::vec3 input, glm::vec3 origin, glm::quat rotation) {
      return origin + rotation * (input - origin);
    }
  };
} //core

