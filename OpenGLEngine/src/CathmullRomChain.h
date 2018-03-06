#pragma once
#include <vector>
#include "glm\glm.hpp"

namespace core
{
  class CathmullRomChain
  {
  public:
    //Creates a CMR chain using the points given
    //Will create a chain of points - 3 splines
    CathmullRomChain(const std::vector<glm::vec3>& points);
    ~CathmullRomChain();
    glm::vec3 GetPoint(const float& time);
    static glm::vec3 CatmullRomSpline(float time, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
  private:
    std::vector<glm::vec3> points_;
    unsigned int num_splines_;
    static const glm::mat4 CatmullRomCoeffs;
  };
}

