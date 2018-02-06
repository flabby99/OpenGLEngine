#pragma once
#include <random>
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
} //namespace core