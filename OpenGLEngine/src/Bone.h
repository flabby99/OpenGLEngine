#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm\glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <vector>
#include <memory>

namespace IK
{
  //Limits for angles, expressed in radians
  struct AngleLimits {
    float pos_x, pos_y , pos_z;
    float neg_x, neg_y, neg_z;
  };

  class Bone
  {
    friend class BoneChain;
  private:
    glm::vec3 start_;
    glm::vec3 end_;
    glm::quat orientation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    AngleLimits angle_limits_;
    std::shared_ptr<Bone> parent_ = nullptr;
    std::shared_ptr<Bone> child_ = nullptr;

    //TODO I need to update GetEndEffector and my orient and parent orients
    //TODO I need to update base to take in only parent orients
    //My orientation can locally be upated in solver, this is fine
  public:
    Bone();
    ~Bone();
    inline Bone(glm::vec3 start, glm::vec3 end) 
      : start_(start), end_(end)
    {}
    inline void SetParent(std::shared_ptr<Bone> parent) {
      parent_ = parent;
      parent_->child_ = std::make_shared<Bone>(this);
    }
    inline const std::shared_ptr<Bone> GetParent() {
      return parent_;
    }
    inline const std::shared_ptr<Bone> GetChild() {
      return child_;
    }
    inline const glm::vec3 GetBase() {
      return orientation_ * start_;
    }
    //TODO add a return of the global orientation
    //SEE object class for the idea: it is
    //parent orientation * your orientation for each parent

  };

  //Stores the head and tail of a DLL of bones
  class BoneChain {
  private:
    std::shared_ptr<Bone> base_bone_;
    std::shared_ptr<Bone> end_bone_;
    std::shared_ptr<Bone> current_bone_ = nullptr;
  public:
    inline BoneChain() {}
    inline BoneChain(std::shared_ptr<Bone> start_bone, std::shared_ptr<Bone> end_bone) :
      base_bone_(start_bone), end_bone_(end_bone) {}

    inline const glm::vec3 GetEndEffector() {
      return end_bone_->end_;
    }
    
    //Move up one bone in the chain
    //If you current_bone is off the chain, start from the bottom of the chain again
    //If the next bone is off the chain, then nullptr is set to the current bone and returned
    const std::shared_ptr<Bone> MoveUpOneBone();
  };
} //namespace IK

