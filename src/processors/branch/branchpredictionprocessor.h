#pragma once

#include "../../ripes_types.h"
#include <memory>

namespace Ripes {

/**
 * @brief The BranchPredictionProcessor class
 * Interface for processors that support branch prediction.
 *
 * Inherited alongside the RipesVSRTLProcessor class
 */
class BranchPredictionProcessor {
public:
  BranchPredictionProcessor() {}
  virtual ~BranchPredictionProcessor() {}

  /**
   * @brief currentInstructionIsBranch
   *
   * @return True if current instruction is any type of branch, false otherwise
   */
  virtual bool currentInstructionIsBranch() = 0;

  /**
   * @briefcurrentInstructionIsConditional
   *
   * @return True if current instruction is a conditional branch, false
   * otherwise
   */
  virtual bool currentInstructionIsConditional() = 0;

  /**
   * @brief currentInstructionImmediate
   *
   * @return The value of the immediate for the current instruction
   */
  virtual VInt currentInstructionImmediate() = 0;

  /**
   * @brief currentInstructionTarget
   *
   * @return Gets the value of the current branch target
   */
  virtual AInt currentInstructionTarget() = 0;

  /**
   * @brief currentGetPrediction
   *
   * @return True if current instruction is predicted taken
   */
  virtual bool currentGetPrediction() = 0;
};

} // namespace Ripes
