#pragma once

#include "../../ripes_types.h"
#include <deque>
#include <memory>

namespace Ripes {

/**
 * @brief The RipesBranchPredictor class
 * Interface for all Ripes branch predictors. This interface is intended to be
 * ISA-agnostic and provides a consistent interface to branch predictor
 * algorithms. This also makes it very easy to add more algorithms to Ripes.
 *
 * Integer values are communicated in uin64_t variables. If the implementing
 * processor implements a narrower register width, e.g., 32 bit, then only the
 * lower 32 bits should be considered.
 */
template <typename XLEN_T, typename ARRAY_T>
class RipesBranchPredictor {
public:
  RipesBranchPredictor() {}
  virtual ~RipesBranchPredictor() {}

  /**
   * @brief getPrediction
   *
   * @param addr           Address of current instruction
   * @param is_branch      True if current instruction is branch
   * @param is_conditional True if current instruction is conditional
   * @return               Whether or not the current instruction is predicted
   *                       to be taken
   */
  virtual bool getPrediction(XLEN_T addr, bool is_branch,
                             bool is_conditional) = 0;

  /**
   * @brief updatePrediction
   *
   * @param addr           Address of previous instruction
   * @param predict_taken  True if previous instruction was predicted taken
   * @param miss           True if previous prediction was wrong
   * @param is_branch      True if previous instruction is branch
   * @param is_conditional True if previous instruction is conditional
   */
  virtual void updatePrediction(XLEN_T addr, bool predict_taken, bool miss,
                                bool is_branch, bool is_conditional) = 0;

  virtual void setReverseStackSize(VInt size) = 0;

  /**
   * @brief saveState/restoreState
   *
   * Reverse feature, makes sure that the state is consistent while reversing
   * the processor
   */
  virtual void saveState() = 0;
  virtual void restoreState() = 0;

  /**
   * @brief resetPredictorState
   *
   * Resets all internal state of the predictor
   */
  virtual void resetPredictorState() = 0;

  void resetPredictorCounters() {
    num_unconditional = 0;
    num_unconditional_miss = 0;
    num_conditional = 0;
    num_conditional_miss = 0;
  }

  double getConditionalAccuracy() {
    double num = num_conditional;
    double num_miss = num_conditional_miss;
    return (1.0 - (double)num_miss / (double)num) * 100.0;
  }

  double getUnconditionalAccuracy() {
    double num = num_unconditional;
    double num_miss = num_unconditional_miss;
    return (1.0 - (double)num_miss / (double)num) * 100.0;
  }

  double getTotalAccuracy() {
    double num = num_conditional + num_unconditional;
    double num_miss = num_conditional_miss + num_unconditional_miss;
    return (1.0 - (double)num_miss / (double)num) * 100.0;
  }

  VInt num_conditional = 0;
  VInt num_conditional_miss = 0;
  VInt num_unconditional = 0;
  VInt num_unconditional_miss = 0;
  VInt rev_stack_max_size = 0;
  std::unique_ptr<ARRAY_T[]> lht;
  std::unique_ptr<ARRAY_T[]> pht;
  bool is_zero_take = false;
};

} // namespace Ripes
