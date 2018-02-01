#include "Camera.h"
#include "glm/gtx/transform.hpp"

//REFERENCE, I updated the mouse update using http://in2gpu.com/2016/03/14/opengl-fps-camera-quaternion/
//Another solution is https://github.com/hmazhar/moderngl_camera/blob/master/camera.cpp
//NOTE the camera currently rolls when the mouse is moved in a circular motion, this is kind of cool but not intended

Camera::Camera()
{
    upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    viewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::quat current_orientation = glm::normalize(glm::angleAxis(0.0f, glm::vec3(1.0f)));
}

Camera::Camera(glm::vec3 pos, glm::vec3 view, glm::vec3 up)
{
    upVector = up;
    position = pos;
    viewDirection = view;
    glm::quat current_orientation = glm::angleAxis(0.0f, glm::vec3(1.0f));
}

glm::mat4 Camera::getMatrix() const
{
    //return glm::lookAt(position, position + viewDirection, upVector);
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), -position);
    glm::mat4 rotate = glm::toMat4(current_orientation);
    return translate * rotate;
}

void Camera::mouseUpdate(const glm::vec2 & newMousePosition)
{
    if (!first_click) {
        first_click = true;
        mousePosition = newMousePosition;
        return;
    }
    glm::vec2 mouseDelta = newMousePosition - mousePosition;
    float key_pitch = -mouseDelta.y * rotation_speed;
    float key_yaw = -mouseDelta.x * rotation_speed;

    glm::quat key_quat = glm::quat(glm::vec3(key_pitch, key_yaw, 0));

    //order matters,update camera_quat
    current_orientation = glm::normalize(key_quat * current_orientation);
    viewDirection = current_orientation * glm::vec3(0.0f, 0.0f, -1.0f);
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
        position += movement_speed * 10 * rightVector * movement.x;
    }
    if (movement.y != 0) position += movement_speed * 10 * viewDirection * movement.y;
    if (movement.z != 0) position += movement_speed * 10 * upVector * movement.z;
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