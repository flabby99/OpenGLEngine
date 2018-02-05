#pragma once
#include <random>

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

} //namespace core