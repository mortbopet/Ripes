#pragma once

#include "ripesbranchpredictor.h"

namespace Ripes {

template <typename XLEN_T, typename ARRAY_T>
class LocalPredictor : public RipesBranchPredictor<XLEN_T, ARRAY_T> {
  static_assert(std::is_same<uint32_t, XLEN_T>::value ||
                    std::is_same<uint64_t, XLEN_T>::value,
                "Only supports 32- and 64-bit variants");
  static constexpr unsigned XLEN = sizeof(XLEN_T) * 8;

public:
  LocalPredictor(VInt num_address_bits, VInt num_history_bits,
                 VInt num_state_bits) {
    this->num_address_bits = num_address_bits;
    this->num_history_bits = num_history_bits;
    this->num_state_bits = num_state_bits;
    this->lht.reset(new ARRAY_T[1 << num_address_bits]());
    this->pht.reset(new ARRAY_T[1 << num_history_bits]());
    resetPredictorState();
  }

  VInt num_address_bits = 0;
  VInt num_history_bits = 0;
  VInt num_state_bits = 0;

  bool getPrediction(XLEN_T addr, bool is_branch,
                     bool is_conditional) override {
    if (is_branch && !is_conditional) {
      return true;
    }

    else if (!is_conditional) {
      return false;
    }

    ARRAY_T check_bits =
        ((addr >> 2) << (XLEN - num_address_bits)) >> (XLEN - num_address_bits);

    if (num_address_bits == 0) {
      check_bits = 0;
    }

    ARRAY_T history = this->lht.get()[check_bits];
    ARRAY_T prediction_state = this->pht.get()[history];

    switch (prediction_state >> (num_state_bits - 1)) {
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
    Q_UNUSED(is_branch);
    if (!is_conditional) {
      return;
    }

    ARRAY_T check_bits =
        ((addr >> 2) << (XLEN - num_address_bits)) >> (XLEN - num_address_bits);

    if (num_address_bits == 0) {
      check_bits = 0;
    }

    bool prev_actual_taken = predict_taken ^ miss;

    ARRAY_T history = this->lht.get()[check_bits];

    this->lht.get()[check_bits] =
        (history | (prev_actual_taken << num_history_bits)) >> 1;

    ARRAY_T prediction_state = this->pht.get()[history];

    if (prev_actual_taken) {
      if (prediction_state == (1 << num_state_bits) - 1) {
        return;
      } else if (prediction_state >= (1 << (num_state_bits - 1)) - 1) {
        this->pht.get()[history] = (1 << num_state_bits) - 1;
      } else {
        this->pht.get()[history] += 1;
      }
    } else {
      if (prediction_state == 0) {
        return;
      } else if (prediction_state <= (1 << (num_state_bits - 1))) {
        this->pht.get()[history] = 0;
      } else {
        this->pht.get()[history] -= 1;
      }
    }
  }

  void setReverseStackSize(VInt size) override {
    if (this->rev_stack_max_size > size) {
      m_rev_lht_stack.resize(size);
      m_rev_pht_stack.resize(size);
    }
  }

  void saveState() override {
    for (int i = 0; i < (1 << num_address_bits); i++) {
      m_rev_lht_stack.push_front(this->lht.get()[i]);
    }

    for (int i = 0; i < (1 << num_history_bits); i++) {
      m_rev_pht_stack.push_front(this->pht.get()[i]);
    }

    if (m_rev_lht_stack.size() >
        this->rev_stack_max_size * (1 << num_address_bits)) {
      for (int i = 0; i < 1 << num_address_bits; i++) {
        m_rev_lht_stack.pop_back();
      }
    }

    if (m_rev_pht_stack.size() >
        this->rev_stack_max_size * (1 << num_history_bits)) {
      for (int i = 0; i < 1 << num_history_bits; i++) {
        m_rev_pht_stack.pop_back();
      }
    }
  }

  void restoreState() override {
    if (m_rev_lht_stack.size() == 0) {
      return;
    }

    if (m_rev_pht_stack.size() == 0) {
      return;
    }

    for (int i = 0; i < (1 << num_address_bits); i++) {
      this->lht.get()[(1 << num_address_bits) - i - 1] =
          m_rev_lht_stack.front();
      m_rev_lht_stack.pop_front();
    }

    for (int i = 0; i < (1 << num_history_bits); i++) {
      this->pht.get()[(1 << num_history_bits) - i - 1] =
          m_rev_pht_stack.front();
      m_rev_pht_stack.pop_front();
    }
  }

  void resetPredictorState() override {
    std::fill_n(this->lht.get(), 1 << num_address_bits, 0);
    std::fill_n(this->pht.get(), 1 << num_history_bits, 0);
  }

private:
  std::deque<ARRAY_T> m_rev_lht_stack;
  std::deque<ARRAY_T> m_rev_pht_stack;
};

} // namespace Ripes