#include "Bone.h"
#include "Maths.h"
#include "Object.h"

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
    if (current_bone_ == base_bone_) {
      current_bone_ = nullptr;
      return current_bone_;
    }
    current_bone_ = current_bone_->parent_;
    if (current_bone_ == nullptr) fprintf(stderr, "WARNING: null parent reached in chain before reaching start of chain");
    return current_bone_;
  }

  std::vector<std::shared_ptr<Bone>> BoneChain::MakeChain(std::vector<glm::vec3> points)
  {
    if (points.size() < 2) { 
      fprintf(stderr, "ERROR not enough points giving to make chain\n");
      exit(-1);
    }
    std::vector<std::shared_ptr<Bone>> bones;
    for (unsigned int i = 0; i < points.size() - 1; ++i) {
      bones.push_back(std::make_shared<Bone>(points[i], points[i + 1]));
      if (i != 0) {
        bones[i-1]->AddChild(bones[i]);
      }
    }
    base_bone_ = bones[0];
    end_bone_ = bones[points.size() - 2];
    return bones;
  }

  //Make a chain and assign a copy of object to each bone in the chain
  std::vector<std::shared_ptr<Bone>> BoneChain::MakeChain(std::vector<glm::vec3> points, std::shared_ptr<scene::Object> object)
  {
    if (points.size() < 2) {
      fprintf(stderr, "ERROR not enough points giving to make chain\n");
      exit(-1);
    }
    std::vector<std::shared_ptr<Bone>> bones;
    bones.reserve(points.size() -1);
    for (unsigned int i = 0; i < points.size() - 1; ++i) {
      bones.push_back(std::make_shared<Bone>(points[i], points[i + 1]));
      std::shared_ptr<scene::Object> mesh = std::make_shared<scene::Object>(*object);
      if (i != 0) {
        bones[i - 1]->AddChild(bones[i]);
      }
      else {
        mesh->SetColour(glm::vec3(1.0f, 0.0f, 1.0f));
      }
      bones[i]->SetObject(mesh);
    }
    base_bone_ = bones[0];
    end_bone_ = bones[points.size() - 2];
    return bones;
  }

  void Bone::UpdateOrientation(glm::quat rotation)
  {
    orientation_ = rotation * orientation_;
    end_ = core::Maths::RotateAroundPoint(end_, start_, rotation);
    UpdateChildren(start_, rotation);
  }

  const glm::quat Bone::GetGlobalOrientation() {
    glm::quat global = orientation_;
      if (parent_ != nullptr) {
        global = parent_->GetGlobalOrientation() * orientation_;
      }
      return global;
  }

  void Bone::SetObject(const std::shared_ptr<scene::Object>& object)
  {
    object_ = object;
    object->SetTranslation(start_);
    object_->UpdateModelMatrix();
  }

  void Bone::UpdateChildren(glm::vec3 origin, glm::quat orientation) {
    if (!children_.empty()) {
      for (std::weak_ptr<Bone>& child : children_) {
        if (auto sp = child.lock()) {
          //Set the childs start to be its parent's end
          sp->start_ = sp->GetParent()->GetEnd();
          //Update the childs end
          sp->end_ = core::Maths::RotateAroundPoint(sp->end_, origin, orientation);
          sp->UpdateChildren(origin, orientation);
        }
      }
    }
  }

  void Bone::UpdateObject() {
    if (object_ != nullptr) {
      object_->SetTranslation(start_);
      object_->UpdateModelMatrix();
    }
    if (!children_.empty()) {
      for (std::weak_ptr<Bone>& child : children_) {
        if (auto sp = child.lock()) {
          sp->UpdateObject();
        }
      }
    }
  }
} //namespace IK