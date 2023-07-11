#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../rv_decode.h"
#include "../rv_immediate.h"

#include <cstdio>

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class BranchUnit : public Component {
public:
  BranchUnit(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    changePredictor(0, NUM_PC_CHECK_BITS, NUM_HISTORY_BITS,
                    NUM_PREDICTION_BITS);

    curr_pre_targ << [=] { return get_curr_pre_targ(); };

    curr_pre_take << [=] {
      (this->*(updateFuncs[predictor]))();
      bool take = (this->*(predictionFuncs[predictor]))();
      return take;
    };

    curr_is_b << [=] { return get_curr_is_b(); };

    curr_is_j << [=] { return get_curr_is_j(); };

    curr_instr >> __decode->instr;
    curr_instr >> __immediate->instr;
    __decode->opcode >> __immediate->opcode;
    dummyreg->out >> dummyreg->in;
  }

  SUBCOMPONENT(__decode, TYPE(Decode<XLEN>));
  SUBCOMPONENT(__immediate, TYPE(Immediate<XLEN>));
  SUBCOMPONENT(dummyreg, Register<1>);

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

  unsigned NUM_PC_CHECK_BITS = 8;
  unsigned NUM_HISTORY_BITS = 8;
  unsigned NUM_PREDICTION_BITS = 2;
  static constexpr unsigned NUM_PREDICTORS = 5;
  std::unique_ptr<uint16_t[]> lht;
  std::unique_ptr<uint16_t[]> pht;
  std::deque<uint16_t> m_reverse_lht_stack;
  std::deque<uint16_t> m_reverse_pht_stack;

  void resetPredictorState() {
    std::fill_n(lht.get(), 1 << NUM_PC_CHECK_BITS, 0);
    std::fill_n(pht.get(), 1 << NUM_HISTORY_BITS, 0);
  }

  void resetPredictorCounters() {
    num_branch = 0;
    num_branch_miss = 0;
  }

  void saveState() {
    for (int i = 0; i < (1 << NUM_PC_CHECK_BITS); i++) {
      m_reverse_lht_stack.push_front(lht.get()[i]);
    }
    for (int i = 0; i < (1 << NUM_HISTORY_BITS); i++) {
      m_reverse_pht_stack.push_front(pht.get()[i]);
    }

    if (m_reverse_lht_stack.size() >
        dummyreg->reverseStackSize() * (1 << NUM_PC_CHECK_BITS)) {
      for (int i = 0; i < 1 << NUM_PC_CHECK_BITS; i++) {
        m_reverse_lht_stack.pop_back();
      }
    }

    if (m_reverse_pht_stack.size() >
        dummyreg->reverseStackSize() * (1 << NUM_HISTORY_BITS)) {
      for (int i = 0; i < 1 << NUM_HISTORY_BITS; i++) {
        m_reverse_pht_stack.pop_back();
      }
    }
  }

  void restoreState() {
    if (m_reverse_lht_stack.size() == 0) {
      return;
    }
    if (m_reverse_pht_stack.size() == 0) {
      return;
    }

    for (int i = 0; i < (1 << NUM_PC_CHECK_BITS); i++) {
      lht.get()[(1 << NUM_PC_CHECK_BITS) - i - 1] = m_reverse_lht_stack.front();
      m_reverse_lht_stack.pop_front();
    }
    for (int i = 0; i < (1 << NUM_HISTORY_BITS); i++) {
      pht.get()[(1 << NUM_HISTORY_BITS) - i - 1] = m_reverse_pht_stack.front();
      m_reverse_pht_stack.pop_front();
    }
  }

  void changePredictor(uint8_t predictor, uint16_t num_pc_check_bits,
                       uint16_t num_history_bits,
                       uint16_t num_prediction_bits) {
    NUM_PC_CHECK_BITS = num_pc_check_bits;
    NUM_HISTORY_BITS = num_history_bits;
    NUM_PREDICTION_BITS = num_prediction_bits;
    this->predictor = predictor;

    lht.reset(new uint16_t[1 << NUM_PC_CHECK_BITS]());
    pht.reset(new uint16_t[1 << NUM_HISTORY_BITS]());

    resetPredictorState();
  }

private:
  uint64_t get_curr_pre_targ() {
    if (get_curr_is_b() || get_curr_is_j()) {
      return curr_pc.uValue() + __immediate->imm.uValue();
    }
    return 0;
  }

  bool get_curr_is_b() {
    Switch(__decode->opcode, RVInstr) {
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
    Switch(__decode->opcode, RVInstr) {
    case RVInstr::JAL:
    case RVInstr::JALR:
      return true;
    default:
      return false;
    }
  }

  bool getPredictionTwoLevel() {
    if (get_curr_is_j()) {
      return true;
    }

    else if (!get_curr_is_b()) {
      return false;
    }

    uint16_t check_bits =
        ((curr_pc.uValue() >> 2) << (64 - NUM_PC_CHECK_BITS)) >>
        (64 - NUM_PC_CHECK_BITS);
    if (NUM_PC_CHECK_BITS == 0) {
      check_bits = 0;
    }
    uint16_t history = lht.get()[check_bits];
    uint16_t prediction_state = pht.get()[history];

    switch (prediction_state >> (NUM_PREDICTION_BITS - 1)) {
    case 0:
      return false;
    case 1:
      return true;
    default:
      return false;
    }
  }

  void updatePredictionTwoLevel() {
    if (!prev_is_b.uValue()) {
      return;
    }

    uint16_t check_bits =
        ((prev_pc.uValue() >> 2) << (64 - NUM_PC_CHECK_BITS)) >>
        (64 - NUM_PC_CHECK_BITS);
    if (NUM_PC_CHECK_BITS == 0) {
      check_bits = 0;
    }
    bool prev_actual_taken = prev_pre_take.uValue() ^ prev_pre_miss.uValue();
    uint16_t history = lht.get()[check_bits];
    lht.get()[check_bits] =
        (history | (prev_actual_taken << NUM_HISTORY_BITS)) >> 1;
    uint16_t prediction_state = pht.get()[history];

    if (prev_actual_taken) {
      if (prediction_state == (1 << NUM_PREDICTION_BITS) - 1) {
        return;
      } else if (prediction_state >= (1 << (NUM_PREDICTION_BITS - 1)) - 1) {
        pht.get()[history] = (1 << NUM_PREDICTION_BITS) - 1;
      } else {
        pht.get()[history] += 1;
      }
    } else {
      if (prediction_state == 0) {
        return;
      } else if (prediction_state <= (1 << (NUM_PREDICTION_BITS - 1))) {
        pht.get()[history] = 0;
      } else {
        pht.get()[history] -= 1;
      }
    }
  }

  bool getPredictionCounter() {
    if (get_curr_is_j()) {
      return true;
    }

    else if (!get_curr_is_b()) {
      return false;
    }

    uint16_t prediction_state = pht.get()[0];

    switch (prediction_state >> (NUM_PREDICTION_BITS - 1)) {
    case 0:
      return false;
    case 1:
      return true;
    default:
      return false;
    }
  }

  void updatePredictionCounter() {
    if (!prev_is_b.uValue()) {
      return;
    }

    bool prev_actual_taken = prev_pre_take.uValue() ^ prev_pre_miss.uValue();
    uint16_t prediction_state = pht.get()[0];

    if (prev_actual_taken) {
      if (prediction_state == (1 << NUM_PREDICTION_BITS) - 1) {
        return;
      } else {
        pht.get()[0] += 1;
      }
    } else {
      if (prediction_state == 0) {
        return;
      } else {
        pht.get()[0] -= 1;
      }
    }
  }

  bool getPredictionAlwaysTaken() {
    if (get_curr_is_j()) {
      return true;
    }

    if (!get_curr_is_b()) {
      return false;
    }

    return true;
  }

  void updatePredictionAlwaysTaken() { return; }

  bool getPredictionAlwaysNotTaken() {
    if (get_curr_is_j()) {
      return true;
    }

    return false;
  }

  void updatePredictionAlwaysNotTaken() { return; }

  uint8_t predictor = 0;

  typedef bool (BranchUnit<XLEN>::*getPredictionFunc)(void);
  typedef void (BranchUnit<XLEN>::*updatePredictionFunc)(void);

  getPredictionFunc predictionFuncs[NUM_PREDICTORS] = {
      &BranchUnit<XLEN>::getPredictionTwoLevel,
      &BranchUnit<XLEN>::getPredictionTwoLevel,
      &BranchUnit<XLEN>::getPredictionCounter,
      &BranchUnit<XLEN>::getPredictionAlwaysTaken,
      &BranchUnit<XLEN>::getPredictionAlwaysNotTaken};

  updatePredictionFunc updateFuncs[NUM_PREDICTORS] = {
      &BranchUnit<XLEN>::updatePredictionTwoLevel,
      &BranchUnit<XLEN>::updatePredictionTwoLevel,
      &BranchUnit<XLEN>::updatePredictionCounter,
      &BranchUnit<XLEN>::updatePredictionAlwaysTaken,
      &BranchUnit<XLEN>::updatePredictionAlwaysNotTaken};
};
} // namespace core
} // namespace vsrtl