#include "CathmullRomChain.h"
#include "Maths.h"


namespace core
{
  CathmullRomChain::CathmullRomChain(const std::vector<glm::vec3>& points)
    : points_(points), num_splines_(points.size() - 3)
  {
    if (num_splines_ < 1) {
      fprintf(stderr, "ERROR: Tried to create a CMR spline without enough points");
    }
  }


  CathmullRomChain::~CathmullRomChain()
  {
  }

  glm::vec3 CathmullRomChain::GetPoint(const float & time)
  {
    float t = time;
    unsigned int spline_index;
    if (t >= (float)num_splines_) {
      fprintf(stderr, "WARNING, time out of range of CMR spline\n");
      t = 1;
      spline_index = num_splines_ - 1;
    }
    else {
      float temp;
      t = modf(t, &temp);
      spline_index = (unsigned int)temp;
    }
    glm::vec3 *p0 = &points_[spline_index];
    glm::vec3 *p1 = &points_[spline_index + 1];
    glm::vec3 *p2 = &points_[spline_index + 2];
    glm::vec3 *p3 = &points_[spline_index + 3];
    return CatmullRomSpline(t, *p0, *p1, *p2, *p3);
  }

  glm::vec3 CathmullRomChain::CatmullRomSpline(float time, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
  {
    glm::mat4x3 points(p0, p1, p2, p3);
    glm::vec4 time_co_effs(1.0f, time, time * time, time * time * time);
    glm::mat4x3 geometric = points * CatmullRomCoeffs;
    auto result = 0.5f * geometric * time_co_effs;
    return result;
  }

}
