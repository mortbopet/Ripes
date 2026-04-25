#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "processors/RISC-V/riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class Wildcat_IDEX : public Component {
public:
  Wildcat_IDEX(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    setDescription("ID/EX stage separating register");
    CONNECT_REGISTERED_CLEN_INPUT(pc, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(pc_4, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(op1, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(op2, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(mem_addr, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(mem_wr_data, clear, enable);

    CONNECT_REGISTERED_CLEN_INPUT(reg_wr_src_ctrl, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(wr_reg_idx, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(reg_do_write, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(mem_do_write, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(alu_ctrl, clear, enable);
    CONNECT_REGISTERED_CLEN_INPUT(mem_op, clear, enable);

    CONNECT_REGISTERED_CLEN_INPUT(valid, clear, enable);
  }

  // Data
  REGISTERED_CLEN_INPUT(pc_4, XLEN);
  REGISTERED_CLEN_INPUT(op1, XLEN);
  REGISTERED_CLEN_INPUT(op2, XLEN);
  REGISTERED_CLEN_INPUT(mem_addr, XLEN);
  REGISTERED_CLEN_INPUT(mem_wr_data, XLEN);

  // Control
  REGISTERED_CLEN_INPUT(reg_wr_src_ctrl, enumBitWidth<RegWrSrc>());
  REGISTERED_CLEN_INPUT(wr_reg_idx, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(reg_do_write, 1);
  REGISTERED_CLEN_INPUT(alu_ctrl, enumBitWidth<ALUOp>());
  REGISTERED_CLEN_INPUT(mem_do_write, 1);
  REGISTERED_CLEN_INPUT(mem_op, enumBitWidth<MemOp>());

  // Interface compliance
  REGISTERED_CLEN_INPUT(pc, XLEN);
  //REGISTERED_CLEN_INPUT(opcode, enumBitWidth<RVInstr>());

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
