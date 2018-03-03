#include "IK_Solver.h"

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
} //namespace IK