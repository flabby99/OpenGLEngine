#pragma once
#include <random>
#include "glew\glew.h"
#include "glm/glm.hpp"

namespace core {
  //return +1 / -1 at random
  inline int rand_sign() {
    int sign;
    (rand() % 2 == 0) ? (sign = -1) : (sign = 1);
    return sign;
  }
  //return a random float in 0 1
  inline float frand() {
    return rand() / (float)RAND_MAX;
  }
  //return a random float in -1 1
  inline float frand_s() {
    return rand_sign() * frand();
  }
  inline glm::vec3 make_vertex3(const int index, const std::vector<GLfloat>& vector) {
    return glm::vec3(vector[index], vector[index + 1], vector[index + 2]);
  }
  inline glm::vec2 make_vertex2(const int index, const std::vector<GLfloat>& vector) {
    return glm::vec2(vector[index], vector[index + 1]);
  }
  inline GLfloat* vec3_array_to_float_array(const glm::vec3* vectors, int num_vectors) {
    GLfloat* float_array = new(num_vectors * 3);
    for (int i = 0; i < num_vectors; ++i) {
      float_array[3 * i] = vectors[i].x;
      float_array[3 * i + 1] = vectors[i].y;
      float_array[3 * i + 2] = vectors[i].z;
    }
    return float_array;
  }
} //namespace core