#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_register.h"

#include "processors/RISC-V/riscv.h"

#include "rv5s_vliw_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

#define AUTO_CONNECT_CLEN_INPUT(input)                                         \
  CONNECT_REGISTERED_CLEN_INPUT(input, clear, enable)

template <unsigned XLEN>
class IDEX_VLIW : public Component {
  /*  This is controversial but inheriting from RV5S_IDEX<XLEN> would force us
      to use the pc4 port instead of a more appropriate named pc8 and also the
      more generic named ctrl lines instead of the exec and data.
      (which would be more consistent with the current design).
      Unless there is a more generic approach we have to do all assignments
      manually again for readability purposes.
  */
public:
  IDEX_VLIW(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    SimBase::setDescription(
        "Instruction issue/execute stage separating register");

    AUTO_CONNECT_CLEN_INPUT(valid);

    // PC
    AUTO_CONNECT_CLEN_INPUT(pc);
    AUTO_CONNECT_CLEN_INPUT(pc8);
    AUTO_CONNECT_CLEN_INPUT(pc_data_offset);

    // Exec Way
    AUTO_CONNECT_CLEN_INPUT(r1_exec);
    AUTO_CONNECT_CLEN_INPUT(r2_exec);
    AUTO_CONNECT_CLEN_INPUT(imm_exec);
    AUTO_CONNECT_CLEN_INPUT(rd_reg1_idx_exec);
    AUTO_CONNECT_CLEN_INPUT(rd_reg2_idx_exec);
    AUTO_CONNECT_CLEN_INPUT(wr_reg_idx_exec);
    AUTO_CONNECT_CLEN_INPUT(reg_do_write_exec);

    // Data Way
    AUTO_CONNECT_CLEN_INPUT(r1_data);
    AUTO_CONNECT_CLEN_INPUT(r2_data);
    AUTO_CONNECT_CLEN_INPUT(imm_data);
    AUTO_CONNECT_CLEN_INPUT(rd_reg1_idx_data);
    AUTO_CONNECT_CLEN_INPUT(rd_reg2_idx_data);
    AUTO_CONNECT_CLEN_INPUT(wr_reg_idx_data);
    AUTO_CONNECT_CLEN_INPUT(reg_do_write_data);

    // Control
    AUTO_CONNECT_CLEN_INPUT(reg_wr_src_exec_ctrl);
    AUTO_CONNECT_CLEN_INPUT(alu_exec_op1_ctrl);
    AUTO_CONNECT_CLEN_INPUT(alu_exec_op2_ctrl);
    AUTO_CONNECT_CLEN_INPUT(alu_exec_ctrl);

    AUTO_CONNECT_CLEN_INPUT(mem_do_write);
    AUTO_CONNECT_CLEN_INPUT(mem_op);
    AUTO_CONNECT_CLEN_INPUT(exec_is_valid);
    AUTO_CONNECT_CLEN_INPUT(data_is_valid);
    AUTO_CONNECT_CLEN_INPUT(br_op);
    AUTO_CONNECT_CLEN_INPUT(do_br);
    AUTO_CONNECT_CLEN_INPUT(do_jmp);
  }

  // Register bank controls
  INPUTPORT(enable, 1);
  INPUTPORT(clear, 1);

  // Valid signal. False when the register bank has been cleared. May be used by
  // UI to determine whether the NOP in the stage is a user-inserted nop or the
  // result of some pipeline action.
  REGISTERED_CLEN_INPUT(valid, 1);

  // PC
  REGISTERED_CLEN_INPUT(pc, XLEN);
  REGISTERED_CLEN_INPUT(pc8, XLEN);
  REGISTERED_CLEN_INPUT(pc_data_offset, XLEN);

  // Exec Way
  REGISTERED_CLEN_INPUT(r1_exec, XLEN);
  REGISTERED_CLEN_INPUT(r2_exec, XLEN);
  REGISTERED_CLEN_INPUT(imm_exec, XLEN);
  REGISTERED_CLEN_INPUT(rd_reg1_idx_exec, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(rd_reg2_idx_exec, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(wr_reg_idx_exec, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(reg_do_write_exec, 1);

  // Data Way
  REGISTERED_CLEN_INPUT(r1_data, XLEN);
  REGISTERED_CLEN_INPUT(r2_data, XLEN);
  REGISTERED_CLEN_INPUT(imm_data, XLEN);
  REGISTERED_CLEN_INPUT(rd_reg1_idx_data, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(rd_reg2_idx_data, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(wr_reg_idx_data, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(reg_do_write_data, 1);

  // Control
  REGISTERED_CLEN_INPUT(reg_wr_src_exec_ctrl, enumBitWidth<RegWrExecSrc>());
  REGISTERED_CLEN_INPUT(alu_exec_op1_ctrl, enumBitWidth<AluSrc1>());
  REGISTERED_CLEN_INPUT(alu_exec_op2_ctrl, enumBitWidth<AluSrc2>());
  REGISTERED_CLEN_INPUT(alu_exec_ctrl, enumBitWidth<ALUOp>());

  REGISTERED_CLEN_INPUT(mem_do_write, 1);
  REGISTERED_CLEN_INPUT(mem_op, enumBitWidth<MemOp>());
  REGISTERED_CLEN_INPUT(exec_is_valid, 1);
  REGISTERED_CLEN_INPUT(data_is_valid, 1);
  REGISTERED_CLEN_INPUT(br_op, enumBitWidth<CompOp>());
  REGISTERED_CLEN_INPUT(do_br, 1);
  REGISTERED_CLEN_INPUT(do_jmp, 1);
};

#undef AUTO_CONNECT_CLEN_INPUT

} // namespace core
} // namespace vsrtl
