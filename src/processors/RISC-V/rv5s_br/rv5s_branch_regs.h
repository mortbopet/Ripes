#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class RV5S_BR_IFID : public Component {
public:
  RV5S_BR_IFID(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    CONNECT_REGISTERED_CLEN_INPUT(curr_pre_targ, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(curr_pre_take, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(curr_is_b, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(curr_is_j, clear, enable);
  }

  // Data
  REGISTERED_CLEN_INPUT(curr_pre_targ, XLEN);
  REGISTERED_CLEN_INPUT(curr_pre_take, 1);
  REGISTERED_CLEN_INPUT(curr_is_b, 1);
  REGISTERED_CLEN_INPUT(curr_is_j, 1);

  // Register bank controls
  INPUTPORT(enable, 1);
  INPUTPORT(clear, 1);
};

template <unsigned XLEN>
class RV5S_BR_IDEX : public Component {
public:
  RV5S_BR_IDEX(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    CONNECT_REGISTERED_CLEN_INPUT(curr_pre_targ, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(curr_pre_take, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(curr_is_b, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(curr_is_j, clear, enable);
  }

  // Data
  REGISTERED_CLEN_INPUT(curr_pre_targ, XLEN);
  REGISTERED_CLEN_INPUT(curr_pre_take, 1);
  REGISTERED_CLEN_INPUT(curr_is_b, 1);
  REGISTERED_CLEN_INPUT(curr_is_j, 1);

  // Register bank controls
  INPUTPORT(enable, 1);
  INPUTPORT(clear, 1);
};

} // namespace core
} // namespace vsrtl
