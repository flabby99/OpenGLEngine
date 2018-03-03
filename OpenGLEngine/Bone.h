#pragma once
#include "glm\glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

namespace IK
{
  struct AngleLimits {
    float pos_x, pos_y, pos_z;
    float neg_x, neg_y, neg_z;
  };
  class Bone
  {
  private:
    glm::vec3 start_;
    glm::vec3 end_;
    glm::quat orientation_; //Start this at the indentity
    AngleLimits angle_limits_;

  public:
    Bone();
    ~Bone();
  };
} //namespace IK

