#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class BranchMiss : public Component {
public:
  BranchMiss(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    curr_miss_1 << [=] {
      return (curr_is_j.uValue() && !curr_equal_targets.uValue()) |
             (!curr_pre_take.uValue() && curr_act_take.uValue() &&
              curr_is_b.uValue()) |
             (curr_pre_take.uValue() && curr_act_take.uValue() &&
              !curr_equal_targets.uValue() && curr_is_b.uValue());
    };

    curr_miss_2 << [=] {
      return (!curr_act_take.uValue() && curr_pre_take.uValue() &&
              curr_is_b.uValue());
    };
  }

  INPUTPORT(curr_equal_targets, 1);
  INPUTPORT(curr_act_take, 1);
  INPUTPORT(curr_pre_take, 1);
  INPUTPORT(curr_is_b, 1);
  INPUTPORT(curr_is_j, 1);
  OUTPUTPORT(curr_miss_1, 1);
  OUTPUTPORT(curr_miss_2, 1);

private:
};
} // namespace core
} // namespace vsrtl