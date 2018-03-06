#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm\glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
#include <vector>
#include <memory>
#include <list>

namespace scene {
  class Object;
}

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
    std::shared_ptr<scene::Object> object_ = nullptr;
    std::list<std::weak_ptr<Bone>> children_;
    void UpdateChildren(glm::vec3 origin, glm::quat orientation);

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
      return start_;
    }
    inline const glm::vec3 GetEnd() {
      return end_;
    }
    inline const glm::quat GetOrientation() {
      return orientation_;
    }
    inline const std::shared_ptr<scene::Object> GetBoneObject() {
      return object_;
    }
    //Set the origin of rotation to be the bones base
    //Then rotate the end, and update every child bone, rotating around the origin

    void UpdateOrientation(glm::quat rotation);
    const glm::quat GetGlobalOrientation();
    void SetObject(const std::shared_ptr<scene::Object>& object);

    //Recursively update your objects and children's objects
    void UpdateObject();
  };

  //Stores the head and tail of a DLL of bones
  class BoneChain {
  private:
    std::shared_ptr<Bone> base_bone_;
    std::shared_ptr<Bone> end_bone_;
    std::shared_ptr<Bone> current_bone_ = nullptr;
  public:
    inline BoneChain() {}
    //Initialises a single linked chain of bones starting with the first point
    inline BoneChain(std::shared_ptr<Bone> start_bone, std::shared_ptr<Bone> end_bone) :
      base_bone_(start_bone), end_bone_(end_bone) {}

    inline const glm::vec3 GetEndEffector() {
      return end_bone_->end_;
    }
    
    inline void UpdateChainObjects() {
      base_bone_->UpdateObject();
    }
    //Move up one bone in the chain
    //If the current_bone is off the chain, start from the bottom of the chain again
    //If the next bone is off the chain, then nullptr is set to the current bone and returned
    const std::shared_ptr<Bone> MoveUpOneBone();
    std::vector<std::shared_ptr<Bone>> MakeChain(std::vector<glm::vec3> points);
    std::vector<std::shared_ptr<Bone>> MakeChain(std::vector<glm::vec3> points, std::shared_ptr<scene::Object> object);
  };
} //namespace IK