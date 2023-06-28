#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"

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
    curr_pre_targ << [=] { return get_curr_pre_targ(); };

    curr_pre_take << [=] { update_prev_prediction(); bool take = get_curr_pre_take(); return take; };

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

  static constexpr unsigned NUM_HISTORY_BITS = 8;
  static constexpr unsigned NUM_PREDICTION_BITS = 2;

  // This branch unit currently uses an (8, 2) correlated predictor.
  // That is, it keeps track of the result of the last 8 branches and uses
  // a 2-bit state machine to predict the next branch.

  // Given the type of the tables, technically this can support up to a
  // (16, 16) predictor, but this would require 256KB of RAM for the
  // predictor, and would be much more latent, this is unrealistic
  // for actual hardware, whereas (8, 2) is very feasible.

  /**
   * n-bit state machine:
   * 
   *       /--T--\
   *       |     v
   *  /-> [11...11] <-T-- [01...11] --\
   *  |       |               ^       |
   *  T       N               T       N
   *  |       v               |       |
   *  |-- [11...10]       [01...10] --|
   *  |       |               ^       |
   *  T       N               T       N
   *  |       v               |       |
   * ...     ...             ...     ...
   *  |       |               ^       |
   *  T       N               T       N
   *  |       v               |       |
   *  |-- [10...01]       [00...01] --|
   *  |       |               ^       |
   *  T       N               T       N
   *  |       v               |       |
   *  \-- [10...00] --N-> [00...00] <-/
   *                       ^     |
   *                       \--N--/
   * 
   * If MSB is 1, branch is taken, otherwise it is not.
   */

  uint16_t local_history_table[1 << NUM_HISTORY_BITS];
  uint16_t branch_prediction_table[1 << NUM_HISTORY_BITS];

  bool get_curr_pre_take() {
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

  void update_prev_prediction() {
    if (!prev_is_b.uValue()) {
      return;
    }

    if (prev_pre_miss.uValue()) {
      num_branch_miss++;
    }

    num_branch++;

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
};
} // namespace core
} // namespace vsrtl