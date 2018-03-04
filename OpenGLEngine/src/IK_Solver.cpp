#define GLM_ENABLE_EXPERIMENTAL
#include "IK_Solver.h"
#include "Bone.h"
#include "Maths.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

namespace IK
{
  CCD_Solver::CCD_Solver(unsigned int max_iterations, float error_tolerance, float required_improvement)
    : max_iterations_(max_iterations), 
    error_tolerance_(error_tolerance), 
    required_iteration_improvement_(required_improvement)
  {
  }

  CCD_Solver::CCD_Solver()
  {
  }

  CCD_Solver::~CCD_Solver()
  {
  }
  void CCD_Solver::Solve(std::shared_ptr<BoneChain> chain, glm::vec3 target)
  {
    unsigned int iterations = 0;
    float improvement = 0.f;
    float distance = glm::distance(chain->GetEndEffector(), target);
    std::shared_ptr<Bone> current_bone = nullptr;
    while ((iterations < this->max_iterations_) && (distance < error_tolerance_)) {
      //Iterate over the chain and perform CCD calculation
      do {
        //Get current end effector position
        glm::vec3 end_effector = chain->GetEndEffector();

        //Get the next bone to use
        current_bone = chain->MoveUpOneBone();

        glm::vec3 bone_base = current_bone->GetBase();
        glm::vec3 base_to_target = target - bone_base;
        glm::vec3 base_to_end = end_effector - bone_base;

        glm::quat desired_rot = core::Maths::RotationBetweenVectors(base_to_end, base_to_target);

        //Apply this rotation to the bone
        current_bone->UpdateOrientation(desired_rot);
      } while (current_bone != nullptr);  
      ++iterations;
      improvement = distance;
      distance = glm::distance(chain->GetEndEffector(), target);
      improvement -= distance;
      if (improvement < required_iteration_improvement_) {
        break;
      }
    }
  }

} //namespace IK