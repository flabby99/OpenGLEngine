#pragma once
#include <memory>

class BoneChain;

namespace IK
{
  //Required parameters are the num iterations and an error tolerance for target position
  //Optional parameters would be to end if no closer to end effector
  class CCD_Solver
  {
  public:
    //Init CCD to run for n iterations, stopping if within certain error, or not improved by
    //A certain amount after an iteration
    CCD_Solver(unsigned int max_iterations, float error_tolerance, float required_improvement);
    CCD_Solver();
    ~CCD_Solver();

    //Solve for a chain to reach an end effector using CCD
    //@param target - target position for the end effector
    //@param chain - the bone chain that should be moved
    //Note that glm has an rotation between vectors function that returns a quat
    //NOTE also has a look at which I can use to finally fix my camera
    void Solve(std::shared_ptr<BoneChain>, glm::vec3 target);

  private:
    unsigned int max_iterations_ = 100;
    float error_tolerance_ = 0.01f;
    float required_iteration_improvement_ = 0.001f;


  };
} //namespace IK