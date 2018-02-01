#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

class Camera
{  
  glm::vec3 upVector;
  glm::vec3 position;
  glm::vec3 viewDirection;
  glm::vec2 mousePosition;
  glm::quat current_orientation;
  //How fast to rotate and move
  const float rotation_speed = 0.001f;
  const float movement_speed = 0.09f;
public:
  bool first_click = false;
  Camera();
  Camera(glm::vec3 pos, glm::vec3 view, glm::vec3 up);
  //Get the view matrix
  glm::mat4 getMatrix() const;
  //Allows view rotations up and down
  void mouseUpdate(const glm::vec2& newMousePosition);
  //Allows forward and back, left and right movement with mouse
  void mouseMove(const glm::vec2& newMousePosition);
  //Allows 3DOF movement in the direction of movement
  void Move(const glm::vec3 movement);
  void updatePosition(const glm::vec3 newPosition);
  void updateDirection(const glm::vec3 newDirection);
  void rotateViewDirection(const glm::mat4 rotation);
  inline glm::mat4 getRotation() const { return glm::toMat4(current_orientation); }
};

