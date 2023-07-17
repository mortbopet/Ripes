#pragma once

#include "ripesbranchpredictor.h"

namespace Ripes {

template <typename XLEN_T, typename ARRAY_T>
class AlwaysTakenPredictor : public RipesBranchPredictor<XLEN_T, ARRAY_T> {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * 8;

public:
  AlwaysTakenPredictor() {
    this->lht.reset(new ARRAY_T[1]);
    this->pht.reset(new ARRAY_T[1]);
    resetPredictorState();
  }

  bool getPrediction(XLEN_T addr, bool is_branch,
                     bool is_conditional) override {
    Q_UNUSED(addr);
    if (is_branch && !is_conditional) {
      return true;
    }

    else if (!is_conditional) {
      return false;
    }

    return true;
  }

  void updatePrediction(XLEN_T addr, bool predict_taken, bool miss,
                        bool is_branch, bool is_conditional) override {
    Q_UNUSED(addr);
    Q_UNUSED(predict_taken);
    Q_UNUSED(miss);
    Q_UNUSED(is_branch);
    Q_UNUSED(is_conditional);
    return;
  }

  void setReverseStackSize(VInt size) override {
    Q_UNUSED(size);
    return;
  }

  void saveState() override { return; }

  void restoreState() override { return; }

  void resetPredictorState() override {
    this->lht.get()[0] = 0;
    this->pht.get()[0] = 0;
  }
};

} // namespace Ripes