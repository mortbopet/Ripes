#pragma once

#include "../ripesbranchpredictor.h"

namespace Ripes {

template <typename XLEN_T, typename ARRAY_T>
class CounterPredictor : public RipesBranchPredictor<XLEN_T, ARRAY_T> {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * 8;

public:
  CounterPredictor(VInt num_state_bits) {
    this->num_state_bits = num_state_bits;
    this->lht.reset(new ARRAY_T[1]);
    this->pht.reset(new ARRAY_T[1]);
    resetPredictorState();
  }

  VInt num_state_bits = 0;

  bool getPrediction(XLEN_T addr, bool is_branch,
                     bool is_conditional) override {
    Q_UNUSED(addr);
    if (is_branch && !is_conditional) {
      return true;
    }

    else if (!is_conditional) {
      return false;
    }

    switch (this->pht.get()[0] >> (num_state_bits - 1)) {
    case 0:
      return false;
    case 1:
      return true;
    default:
      return false;
    }
  }

  void updatePrediction(XLEN_T addr, bool predict_taken, bool miss,
                        bool is_branch, bool is_conditional) override {
    Q_UNUSED(addr);
    Q_UNUSED(is_branch);
    if (!is_conditional) {
      return;
    }

    bool prev_actual_taken = predict_taken ^ miss;

    if (prev_actual_taken) {
      if (this->pht.get()[0] == (1 << num_state_bits) - 1) {
        return;
      } else {
        this->pht.get()[0] += 1;
      }
    } else {
      if (this->pht.get()[0] == 0) {
        return;
      } else {
        this->pht.get()[0] -= 1;
      }
    }
  }

  void setReverseStackSize(VInt size) override {
    if (this->rev_stack_max_size > size) {
      m_rev_state_stack.resize(size);
    }
  }

  void saveState() override {
    m_rev_state_stack.push_front(this->pht.get()[0]);

    if (m_rev_state_stack.size() > this->rev_stack_max_size) {
      m_rev_state_stack.pop_back();
    }
  }

  void restoreState() override {
    if (m_rev_state_stack.size() == 0) {
      return;
    }

    this->pht.get()[0] = m_rev_state_stack.front();
    m_rev_state_stack.pop_front();
  }

  void resetPredictorState() override {
    this->lht.get()[0] = 0;
    this->pht.get()[0] = 0;
  }

private:
  std::deque<ARRAY_T> m_rev_state_stack;
};

} // namespace Ripes