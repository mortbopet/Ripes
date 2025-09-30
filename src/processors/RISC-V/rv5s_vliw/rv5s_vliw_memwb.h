#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "processors/RISC-V/riscv.h"

#include "rv5s_vliw_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class MEMWB_VLIW : public Component {
public:
  MEMWB_VLIW(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    setDescription("Memory/write-back stage separating register");
    CONNECT_REGISTERED_INPUT(pc);
    CONNECT_REGISTERED_INPUT(pc8);
    CONNECT_REGISTERED_INPUT(pc_data_offset);
    CONNECT_REGISTERED_INPUT(alu_exec_res);
    CONNECT_REGISTERED_INPUT(mem_read);

    CONNECT_REGISTERED_INPUT(reg_wr_src_exec_ctrl);
    CONNECT_REGISTERED_INPUT(wr_reg_idx_exec);
    CONNECT_REGISTERED_INPUT(reg_do_write_exec);
    
    CONNECT_REGISTERED_INPUT(wr_reg_idx_data);
    CONNECT_REGISTERED_INPUT(reg_do_write_data);

    CONNECT_REGISTERED_INPUT(mem_op);
    CONNECT_REGISTERED_INPUT(exec_is_valid);
    CONNECT_REGISTERED_INPUT(data_is_valid);

    CONNECT_REGISTERED_INPUT(valid);
  }

  // Data
  REGISTERED_INPUT(pc, XLEN);
  REGISTERED_INPUT(pc8, XLEN);
  REGISTERED_INPUT(pc_data_offset, XLEN);
  REGISTERED_INPUT(alu_exec_res, XLEN);
  REGISTERED_INPUT(mem_read, XLEN);

  // Control
  REGISTERED_INPUT(reg_wr_src_exec_ctrl, enumBitWidth<RegWrExecSrc>());
  REGISTERED_INPUT(wr_reg_idx_exec, c_RVRegsBits);
  REGISTERED_INPUT(reg_do_write_exec, 1);

  REGISTERED_INPUT(wr_reg_idx_data, c_RVRegsBits);
  REGISTERED_INPUT(reg_do_write_data, 1);

  REGISTERED_INPUT(mem_op, enumBitWidth<MemOp>());
  REGISTERED_INPUT(exec_is_valid, 1);
  REGISTERED_INPUT(data_is_valid, 1);

  // Valid signal. False when the register bank has been cleared. May be used by
  // UI to determine whether the NOP in the stage is a user-inserted nop or the
  // result of some pipeline action.
  REGISTERED_INPUT(valid, 1);
};

} // namespace core
} // namespace vsrtl
