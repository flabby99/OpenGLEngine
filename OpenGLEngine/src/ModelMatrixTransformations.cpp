#define GLM_ENABLE_EXPERIMENTAL
#include "ModelMatrixTransformations.h"
#include "glm/gtx/euler_angles.hpp"

//Handles matrices for transformations
//This could instead be done using glm, but I wanted to make sure I understood the maths
//Designed to be combined with keyboard or mouse input

void ModelMatrixTransformations::UpdateScale(glm::vec3 addscale) {
	scale += addscale;
	scalematrix = glm::mat4(
		scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

void ModelMatrixTransformations::UpdateRotate(glm::vec3 addrotate, bool use_quaternions) {
	if (use_quaternions) {
		glm::quat rotation_quat = glm::angleAxis(0.0f, glm::vec3(1.0f));
		if (addrotate.y != 0) {
			glm::vec3 up(0.0f, 1.0f, 0.0f);
			rotation_quat = glm::normalize(glm::angleAxis(addrotate.y, current_orientation * up));
			current_orientation = glm::normalize(rotation_quat * current_orientation);
		}
		if (addrotate.x != 0) {
			glm::vec3 right(1.0f, 0.0f, 0.0f);
			rotation_quat = glm::normalize(rotation_quat * glm::angleAxis(addrotate.x, current_orientation * right));
			current_orientation = glm::normalize(rotation_quat * current_orientation);
		}
		if (addrotate.z != 0) {
			glm::vec3 forward(0.0f, 0.0f, 1.0f);
			rotation_quat = glm::normalize(rotation_quat * glm::angleAxis(addrotate.z, current_orientation * forward));
			current_orientation = glm::normalize(rotation_quat * current_orientation);
		}
		rotatematrix = glm::toMat4(current_orientation);
	}
	else {
		rotate += addrotate;
		rotate.x = fmod(rotate.x, FULLROTATIONINRADIANS);
		rotate.y = fmod(rotate.y, FULLROTATIONINRADIANS);
		rotate.z = fmod(rotate.z, FULLROTATIONINRADIANS);
		rotatematrix = glm::eulerAngleYXZ(rotate.y, rotate.x, rotate.z);
	}
}

void ModelMatrixTransformations::UpdateTranslate(glm::vec3 addtranslate) {
	translate += addtranslate;
	translatematrix = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		translate.x, translate.y, translate.z, 1.0f);
}

glm::mat4 ModelMatrixTransformations::GetRotationMatrix()
{
	return rotatematrix;
}

//Order of operations is scale first, then rotate, then translate
glm::mat4 ModelMatrixTransformations::UpdateModelMatrix() {
	glm::mat4 tempmatrix = translatematrix * rotatematrix * scalematrix * modelmatrix;
	return tempmatrix;
}

ModelMatrixTransformations::ModelMatrixTransformations() {
	UpdateRotate(glm::vec3(0.0f), false);
	UpdateScale(glm::vec3(0.0f));
	UpdateTranslate(glm::vec3(0.0f));
	UpdateModelMatrix();
}

ModelMatrixTransformations::ModelMatrixTransformations(glm::vec3 translate, glm::vec3 rotate, glm::vec3 scale)
{
	UpdateRotate(rotate, false);
	UpdateScale(scale);
	UpdateTranslate(translate);
	UpdateModelMatrix();
}
