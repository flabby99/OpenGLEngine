#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#define FULLROTATIONINRADIANS 6.28319f

class ModelMatrixTransformations {
  //Matrices are in column-major order
  glm::vec3 scale = glm::vec3(1.0f);
  glm::vec3 rotate = glm::vec3(0.0f);
  glm::vec3 translate = glm::vec3(0.0f);
  glm::mat4 modelmatrix = glm::mat4(1.0f);
  glm::mat4 scalematrix;
  glm::mat4 rotatematrix;
  glm::mat4 translatematrix;
  glm::quat current_orientation = glm::angleAxis(0.0f, glm::vec3(1.0f));
public:
  ModelMatrixTransformations();
  ModelMatrixTransformations(glm::vec3 translate, glm::vec3 rotate, glm::vec3 scale);
  void UpdateScale(glm::vec3 addscale);
  //Angles are in radians.
  void UpdateRotate(glm::vec3 addrotate, bool use_quaternions);
  void UpdateTranslate(glm::vec3 addtranslate);
  glm::mat4 GetRotationMatrix();
  glm::mat4 UpdateModelMatrix();
};

