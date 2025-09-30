#pragma once

#include "../rv5s/rv5s_exmem.h"
#include "rv5s_vliw_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

#define AUTO_CONNECT_CLEN_INPUT(input)  CONNECT_REGISTERED_CLEN_INPUT(input, clear, enable)

template <unsigned XLEN>
class EXMEM_VLIW : public Component {
public:
  EXMEM_VLIW(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    setDescription("Execute/memory stage separating register");

    AUTO_CONNECT_CLEN_INPUT(valid);
    
    // PC
    AUTO_CONNECT_CLEN_INPUT(pc);
    AUTO_CONNECT_CLEN_INPUT(pc8);
    AUTO_CONNECT_CLEN_INPUT(pc_data_offset);

    // Exec Way
    AUTO_CONNECT_CLEN_INPUT(alu_exec_res);
    AUTO_CONNECT_CLEN_INPUT(wr_reg_idx_exec);
    AUTO_CONNECT_CLEN_INPUT(reg_do_write_exec);
    
    // Data Way
    AUTO_CONNECT_CLEN_INPUT(alu_data_res);
    AUTO_CONNECT_CLEN_INPUT(wr_reg_idx_data);
    AUTO_CONNECT_CLEN_INPUT(reg_do_write_data);
    AUTO_CONNECT_CLEN_INPUT(r2_data);

    // Control
    AUTO_CONNECT_CLEN_INPUT(reg_wr_src_exec_ctrl);
    AUTO_CONNECT_CLEN_INPUT(mem_do_write);
    AUTO_CONNECT_CLEN_INPUT(mem_op);
    AUTO_CONNECT_CLEN_INPUT(exec_is_valid);
    AUTO_CONNECT_CLEN_INPUT(data_is_valid);
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
  REGISTERED_CLEN_INPUT(alu_exec_res, XLEN);
  REGISTERED_CLEN_INPUT(wr_reg_idx_exec, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(reg_do_write_exec, 1);
  
  // Data Way
  REGISTERED_CLEN_INPUT(alu_data_res, XLEN);
  REGISTERED_CLEN_INPUT(wr_reg_idx_data, c_RVRegsBits);
  REGISTERED_CLEN_INPUT(reg_do_write_data, 1);
  REGISTERED_CLEN_INPUT(r2_data, XLEN);

  // Control
  REGISTERED_CLEN_INPUT(reg_wr_src_exec_ctrl, enumBitWidth<RegWrExecSrc>());
  REGISTERED_CLEN_INPUT(mem_do_write, 1);
  REGISTERED_CLEN_INPUT(mem_op, enumBitWidth<MemOp>());
  REGISTERED_CLEN_INPUT(exec_is_valid, 1);
  REGISTERED_CLEN_INPUT(data_is_valid, 1);
};

#undef AUTO_CONNECT_CLEN_INPUT

} // namespace core
} // namespace vsrtl
