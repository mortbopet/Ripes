#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../../branch/predictorhandler.h"
#include "../rv_decode.h"
#include "../rv_immediate.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T, unsigned XLEN>
class BranchUnit : public Component {
public:
  BranchUnit(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    changePredictor(0, 8, 8, 2);

    curr_pre_targ << [=] { return get_curr_pre_targ(); };

    curr_pre_take << [=] {
      predictor_ptr.get()->updatePrediction(
          prev_pc.uValue(), prev_pre_take.uValue(), prev_pre_miss.uValue(),
          prev_is_b.uValue(), prev_is_b.uValue());
      bool take = predictor_ptr.get()->getPrediction(
          curr_pc.uValue(), get_curr_is_b() || get_curr_is_j(),
          get_curr_is_b());
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

  std::unique_ptr<RipesBranchPredictor<XLEN_T, uint16_t>> predictor_ptr;

  uint8_t predictor = 0;

  void changePredictor(uint8_t predictor, uint16_t num_address_bits,
                       uint16_t num_history_bits, uint16_t num_state_bits) {
    switch (predictor) {
    case 0:
      predictor_ptr.reset(new LocalPredictor<XLEN_T, uint16_t>(
          num_address_bits, num_history_bits, num_state_bits));
      break;
    case 1:
      predictor_ptr.reset(new GlobalPredictor<XLEN_T, uint16_t>(
          num_history_bits, num_state_bits));
      break;
    case 2:
      predictor_ptr.reset(
          new CounterPredictor<XLEN_T, uint16_t>(num_state_bits));
      break;
    case 3:
      predictor_ptr.reset(new AlwaysTakenPredictor<XLEN_T, uint16_t>());
      break;
    case 4:
      predictor_ptr.reset(new AlwaysNotTakenPredictor<XLEN_T, uint16_t>());
      break;
    }
    this->predictor = predictor;
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
};
} // namespace core
} // namespace vsrtl