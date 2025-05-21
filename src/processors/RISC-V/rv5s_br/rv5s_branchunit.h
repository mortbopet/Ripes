#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../../../processorhandler.h"
#include "../../branch/branchpredictionprocessor.h"
#include "../../branch/predictorhandler.h"
#include "../rv_decode.h"
#include "../rv_immediate.h"

#include <cstdio>

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
  }

  INPUTPORT(prev_is_b, 1);
  INPUTPORT(prev_pc, XLEN);
  INPUTPORT(prev_pre_take, 1);
  INPUTPORT(prev_pre_miss, 1);
  INPUTPORT(curr_pc, XLEN);
  OUTPUTPORT(curr_pre_targ, XLEN);
  OUTPUTPORT(curr_pre_take, 1);
  OUTPUTPORT(curr_is_b, 1);
  OUTPUTPORT(curr_is_j, 1);

  void setProc(BranchPredictionProcessor *p) { this->proc = p; }

private:
  BranchPredictionProcessor *proc = nullptr;
  AInt get_curr_pre_targ() { return proc->currentInstructionTarget(); }

  bool get_curr_is_b() { return proc->currentInstructionIsConditional(); }

  bool get_curr_is_j() {
    return proc->currentInstructionIsBranch() &&
           !proc->currentInstructionIsConditional();
  }
};
} // namespace core
} // namespace vsrtl