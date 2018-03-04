#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm\glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <vector>
#include <memory>
#include <list>

namespace IK
{
  //Limits for angles, expressed in radians
  struct AngleLimits {
    float pos_x, pos_y, pos_z;
    float neg_x, neg_y, neg_z;
  };

  class BoneChain;

  class Bone 
    : public std::enable_shared_from_this<Bone>
  {
    friend class BoneChain;
  private:
    glm::vec3 start_;
    glm::vec3 end_;
    glm::quat orientation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    AngleLimits angle_limits_;
    std::shared_ptr<Bone> parent_ = nullptr;
    std::list<std::weak_ptr<Bone>> children_;

  public:
    Bone();
    ~Bone();
    inline Bone(glm::vec3 start, glm::vec3 end)
      : start_(start), end_(end)
    {}
    inline void AddChild(std::shared_ptr<Bone> child) {
        children_.push_back(child);
        child->parent_ = shared_from_this();
    }
    inline const std::shared_ptr<Bone> GetParent() {
      return parent_;
    }
    inline const std::list<std::weak_ptr<Bone>> GetChildren() {
      return children_;
    }
    inline const glm::vec3 GetBase() {
      if (parent_ != nullptr)
        return parent_->GetGlobalOrientation() * start_;
      else
        return start_;
    }
    inline const void UpdateOrientation(glm::quat rotation) {
      orientation_ = rotation * orientation_;
    }
    const glm::quat GetGlobalOrientation();
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
      return end_bone_->GetGlobalOrientation() * end_bone_->end_;
    }
    //Move up one bone in the chain
    //If the current_bone is off the chain, start from the bottom of the chain again
    //If the next bone is off the chain, then nullptr is set to the current bone and returned
    const std::shared_ptr<Bone> MoveUpOneBone();
  };
} //namespace IK