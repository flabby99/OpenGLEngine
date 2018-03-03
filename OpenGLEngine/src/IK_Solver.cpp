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
    //Could do one step involving the chain as moving current bone towards end_effector:
    //Iterate over the chain and perform CCD calculation
    glm::vec3 end_effector = chain->GetEndEffector();

    //Get the next bone to use
    std::shared_ptr<Bone> current_bone = chain->MoveUpOneBone();

    glm::vec3 bone_base = current_bone->GetBase();
    glm::vec3 base_to_target = target - bone_base;
    glm::vec3 base_to_end = end_effector - bone_base;

    glm::quat desired_rot = core::Maths::RotationBetweenVectors(base_to_end, base_to_target);

    //Apply this rotation to the bone

  }
} //namespace IK