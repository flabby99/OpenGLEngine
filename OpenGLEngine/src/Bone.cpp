#include "Bone.h"

namespace IK
{
  Bone::Bone()
  {
  }


  Bone::~Bone()
  {
  }

  const std::shared_ptr<Bone> BoneChain::MoveUpOneBone() {
    if (!current_bone_) {
      current_bone_ = end_bone_;
      return current_bone_;
    }
    if (current_bone_ == end_bone_) {
      current_bone_ = nullptr;
      return current_bone_;
    }
    current_bone_ = current_bone_->parent_;
    if (current_bone_ == nullptr) fprintf(stderr, "WARNING: null parent reached in chain before reaching start of chain");
    return current_bone_;
  }

  const glm::quat Bone::GetGlobalOrientation() {
    glm::quat global = orientation_;
      if (parent_ != nullptr) {
        global = parent_->GetGlobalOrientation() * orientation_;
      }
      return global;
  }
} //namespace IK
