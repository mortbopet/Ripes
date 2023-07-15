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
    PredictorHandler::changePredictor(0, 8, 8, 2);

    curr_pre_targ << [=] { return get_curr_pre_targ(); };

    curr_pre_take << [=] {
      PredictorHandler::getPredictor()->updatePrediction(
          prev_pc.uValue(), prev_pre_take.uValue(), prev_pre_miss.uValue(),
          prev_is_b.uValue(), prev_is_b.uValue());
      bool take = PredictorHandler::getPredictor()->getPrediction(
          curr_pc.uValue(), get_curr_is_b() || get_curr_is_j(),
          get_curr_is_b());
      return take;
    };

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