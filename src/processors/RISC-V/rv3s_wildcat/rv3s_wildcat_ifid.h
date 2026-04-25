#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "processors/RISC-V/riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class Wildcat_IFID : public Component {
public:
  Wildcat_IFID(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    setDescription("IF/ID stage separating register");
    CONNECT_REGISTERED_CLEN_INPUT(pc, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(pc_4, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(instr, clear, enable);

    CONNECT_REGISTERED_CLEN_INPUT(valid, clear, enable);
  }

  // Data
  REGISTERED_CLEN_INPUT(pc, XLEN);
  REGISTERED_CLEN_INPUT(pc_4, XLEN);
  REGISTERED_CLEN_INPUT(instr, XLEN);


  // Register bank controls
  INPUTPORT(enable, 1);
  INPUTPORT(clear, 1);

  // Valid signal. False when the register bank has been cleared. May be used by
  // UI to determine whether the NOP in the stage is a user-inserted nop or the
  // result of some pipeline action.
  REGISTERED_CLEN_INPUT(valid, 1);
};

} // namespace core
} // namespace vsrtl
