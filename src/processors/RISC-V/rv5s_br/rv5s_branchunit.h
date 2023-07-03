#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"

#include "../rv_decode.h"
#include "../rv_immediate.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class BranchUnit : public Component {
public:
  BranchUnit(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    curr_pre_targ << [=] { return get_curr_pre_targ(); };

    curr_pre_take << [=] { (this->*(updateFuncs[predictor]))(); 
                           bool take = (this->*(predictionFuncs[predictor]))(); 
                           return take; };

    curr_is_b << [=] { return get_curr_is_b(); };

    curr_is_j << [=] { return get_curr_is_j(); };

    curr_instr >> __decode->instr;
    curr_instr >> __immediate->instr;
    __decode->opcode >> __immediate->opcode;
  }

  SUBCOMPONENT(__decode, TYPE(Decode<XLEN>));
  SUBCOMPONENT(__immediate, TYPE(Immediate<XLEN>));

  INPUTPORT(prev_is_b, 1);
  INPUTPORT(prev_pc, XLEN);
  INPUTPORT(prev_pre_take, 1);
  INPUTPORT(prev_pre_miss, 1);
  INPUTPORT(curr_instr, c_RVInstrWidth);
  INPUTPORT(curr_pc, XLEN);
  OUTPUTPORT(curr_pre_targ, XLEN);
  OUTPUTPORT(curr_pre_take, 1);
  OUTPUTPORT(curr_is_b, 1);
  OUTPUTPORT(curr_is_j, 1);

  uint16_t num_branch_miss = 0;
  uint16_t num_branch = 0;

  static constexpr unsigned NUM_HISTORY_BITS = 8;
  static constexpr unsigned NUM_PREDICTION_BITS = 2;
  static constexpr unsigned NUM_PREDICTORS = 1;
  static constexpr unsigned REV_STACK_SIZE = 100;
  uint16_t local_history_table[1 << NUM_HISTORY_BITS];
  uint16_t branch_prediction_table[1 << NUM_HISTORY_BITS];
  std::deque<uint16_t> m_reverse_lht_stack;
  std::deque<uint16_t> m_reverse_bpt_stack;

  void resetPredictorState() {
    for (int i = 0; i < 1 << NUM_HISTORY_BITS; i++) {
      local_history_table[i] = 0;
      branch_prediction_table[i] = 0;
    }
  }

  void resetPredictorCounters() {
    num_branch = 0;
    num_branch_miss = 0;
  }

  void saveState() {
    for (int i = 0; i < (1 << NUM_HISTORY_BITS); i++) {
      m_reverse_lht_stack.push_front(local_history_table[i]);
      m_reverse_bpt_stack.push_front(branch_prediction_table[i]);
    }

    if (m_reverse_lht_stack.size() > REV_STACK_SIZE * (1 << NUM_HISTORY_BITS)) {
      for (int i = 0; i < 1 << NUM_HISTORY_BITS; i++) {
        m_reverse_lht_stack.pop_back();
      }
    }

    if (m_reverse_bpt_stack.size() > REV_STACK_SIZE * (1 << NUM_HISTORY_BITS)) {
      for (int i = 0; i < 1 << NUM_HISTORY_BITS; i++) {
        m_reverse_bpt_stack.pop_back();
      }
    }
  }

  void restoreState() {
    if (m_reverse_lht_stack.size() == 0) {
      return;
    }
    if (m_reverse_bpt_stack.size() == 0) {
      return;
    }

    for (int i = 0; i < (1 << NUM_HISTORY_BITS); i++) {
      local_history_table[(1 << NUM_HISTORY_BITS) - i - 1] = m_reverse_lht_stack.front();
      branch_prediction_table[(1 << NUM_HISTORY_BITS) - i - 1] = m_reverse_bpt_stack.front();
      m_reverse_lht_stack.pop_front();
      m_reverse_bpt_stack.pop_front();
    }
  }

private:

  uint64_t get_curr_pre_targ() {
    if (get_curr_is_b() || get_curr_is_j()) {
      return curr_pc.uValue() + __immediate->imm.uValue();
    }
    return 0;
  }

  bool get_curr_is_b() {
      Switch (__decode->opcode, RVInstr) {
        case RVInstr::BEQ:
        case RVInstr::BNE:
        case RVInstr::BLT:
        case RVInstr::BGE:
        case RVInstr::BLTU:
        case RVInstr::BGEU:
          return true;
        default:
          return false;
      }
  }

  bool get_curr_is_j() {
      Switch (__decode->opcode, RVInstr) {
        case RVInstr::JAL:
          return true;
        case RVInstr::JALR:
          return true;
        default:
          return false;
      }
  }

  bool getPredictionLocal() {
    if (get_curr_is_j()) {
      return true;
    }

    else if (!get_curr_is_b()) {
      return false;
    }

    uint16_t check_bits = ((curr_pc.uValue() >> 2) << (64 - NUM_HISTORY_BITS)) >> (64 - NUM_HISTORY_BITS);
    uint16_t history = local_history_table[check_bits];
    uint16_t prediction_state = branch_prediction_table[history];

    switch (prediction_state >> (NUM_PREDICTION_BITS - 1)) {
      case 0: return false;
      case 1: return true;
      default: return false;
    }
  }

  void updatePredictionLocal() {
    if (!prev_is_b.uValue()) {
      return;
    }

    uint16_t check_bits = ((prev_pc.uValue() >> 2) << (64 - NUM_HISTORY_BITS)) >> (64 - NUM_HISTORY_BITS);
    bool prev_actual_taken = prev_pre_take.uValue() ^ prev_pre_miss.uValue();
    uint16_t history = local_history_table[check_bits];
    local_history_table[check_bits] = (history | (prev_actual_taken << NUM_HISTORY_BITS)) >> 1;
    uint16_t prediction_state = branch_prediction_table[history];

    if (prev_actual_taken) {
      if (prediction_state == (1 << NUM_PREDICTION_BITS) - 1) {
        return;
      }
      else if (prediction_state >= (1 << (NUM_PREDICTION_BITS - 1)) - 1) {
        branch_prediction_table[history] = (1 << NUM_PREDICTION_BITS) - 1;
      }
      else {
        branch_prediction_table[history] += 1;
      }
    }
    else {
      if (prediction_state == 0) {
        return;
      }
      else if (prediction_state <= (1 << (NUM_PREDICTION_BITS - 1))) {
        branch_prediction_table[history] = 0;
      }
      else {
        branch_prediction_table[history] -= 1;
      }
    }
  }

  uint8_t predictor = 0;

  typedef bool (BranchUnit<XLEN>::*getPredictionFunc)(void);
  typedef void (BranchUnit<XLEN>::*updatePredictionFunc)(void);

  getPredictionFunc predictionFuncs[NUM_PREDICTORS] = {
    &BranchUnit<XLEN>::getPredictionLocal
  };

  updatePredictionFunc updateFuncs[NUM_PREDICTORS] = {
    &BranchUnit<XLEN>::updatePredictionLocal
  };
};
} // namespace core
} // namespace vsrtl