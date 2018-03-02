#include "Camera.h"
#include "glm/gtx/transform.hpp"

Camera::Camera()
{
  upVector = glm::vec3(0.0f, 1.0f, 0.0f);
  position = glm::vec3(0.0f, 0.0f, 0.0f);
  viewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
}

Camera::Camera(glm::vec3 pos, glm::vec3 view, glm::vec3 up)
{
  upVector = up;
  position = pos;
  viewDirection = view;
}

glm::mat4 Camera::getMatrix() const
{
  return glm::lookAt(position, position + viewDirection, upVector);
}

void Camera::mouseUpdate(const glm::vec2 & newMousePosition)
{
  if (!first_click) {
    first_click = true;
    mousePosition = newMousePosition;
    return;
  }
  glm::vec2 mouseDelta = newMousePosition - mousePosition;
  viewDirection = glm::normalize(glm::mat3(glm::rotate(-mouseDelta.x * rotation_speed, upVector)) * viewDirection);

  glm::vec3 rightVector = glm::normalize(glm::cross(viewDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
  viewDirection = glm::normalize(glm::mat3(glm::rotate(-mouseDelta.y * rotation_speed, rightVector)) * viewDirection);
  upVector = glm::normalize(glm::cross(rightVector, viewDirection));
  mousePosition = newMousePosition;
}

void Camera::mouseMove(const glm::vec2 & newMousePosition)
{
  if (!first_click) {
    first_click = true;
    mousePosition = newMousePosition;
    return;
  }
  glm::vec2 mouseDelta = newMousePosition - mousePosition;
  position += mouseDelta.y * movement_speed * viewDirection;
  glm::vec3 rightVector = glm::normalize(glm::cross(viewDirection, upVector));
  position += -mouseDelta.x * movement_speed * 0.5f * rightVector;
  mousePosition = newMousePosition;
}

void Camera::Move(const glm::vec3 movement) {
  if (movement.x != 0) {
    glm::vec3 rightVector = glm::normalize(glm::cross(viewDirection, upVector));
    position += movement_speed * 4 * rightVector * movement.x;
  }
  if (movement.y != 0) position += movement_speed * 4 * viewDirection * movement.y;
  if (movement.z != 0) position += movement_speed * 4 * upVector * movement.z;
}

void Camera::updatePosition(const glm::vec3 newPosition)
{
  position = newPosition;
}

void Camera::updateDirection(const glm::vec3 newDirection)
{
  viewDirection = newDirection;
}

void Camera::rotateViewDirection(const glm::mat4 rotation) {
  viewDirection = glm::vec3(rotation * glm::vec4(viewDirection, 1.0f));
}




