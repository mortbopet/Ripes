#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class BranchNextPC : public Component {
public:
  BranchNextPC(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    curr_next << [=] {
      if (curr_miss_1.uValue()) {
        return curr_act_targ.uValue();
      }
      else if (curr_miss_2.uValue()) {
        return pc_4_ex.uValue();
      }
      else if (curr_pre_take.uValue()) {
        return curr_pre_targ.uValue();
      }
      else {
        return pc_4_id.uValue();
      }
    };
  }

  INPUTPORT(curr_pre_targ, XLEN);
  INPUTPORT(pc_4_id, XLEN);
  INPUTPORT(pc_4_ex, XLEN);
  INPUTPORT(curr_act_targ, XLEN);
  INPUTPORT(curr_pre_take, 1);
  INPUTPORT(curr_miss_1, 1);
  INPUTPORT(curr_miss_2, 1);
  OUTPUTPORT(curr_next, XLEN);

private:
};
} // namespace core
} // namespace vsrtl