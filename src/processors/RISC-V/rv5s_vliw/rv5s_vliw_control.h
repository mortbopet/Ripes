#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "processors/RISC-V/rv_control.h"
#include "rv5s_vliw_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class Control_VLIW : public Component {
  static RegWrExecSrc do_reg_wr_src_exec_ctrl(RVInstr opc) {
    switch (opc) {
    // Jump instructions
    case RVInstr::JALR:
    case RVInstr::JAL:
      return RegWrExecSrc::PC8;

    default:
      return RegWrExecSrc::ALURES;
    }
  }

  static inline bool isMemInstr(RVInstr instr) {
    return Control::do_do_read_ctrl(instr) ||
           Control::do_do_mem_write_ctrl(instr);
  }
  static bool isValidExec(RVInstr opc) {
    return opc == RVInstr::NOP || !isMemInstr(opc);
  }
  static bool isValidData(RVInstr opc) {
    return opc == RVInstr::NOP || isMemInstr(opc);
  }

public:
  Control_VLIW(const std::string &name, SimComponent *parent)
      : Component(name, parent) {

    reg_wr_src_exec_ctrl << [this] {
      return do_reg_wr_src_exec_ctrl(opcode_exec.eValue<RVInstr>());
    };

    reg_do_write_exec_ctrl << [this] {
      return isValidExec(opcode_exec.eValue<RVInstr>()) &&
             Control::do_reg_do_write_ctrl(opcode_exec.eValue<RVInstr>());
    };
    reg_do_write_data_ctrl << [this] {
      return isValidData(opcode_data.eValue<RVInstr>()) &&
             Control::do_reg_do_write_ctrl(opcode_data.eValue<RVInstr>());
    };

    mem_do_write_ctrl << [this] {
      return Control::do_do_mem_write_ctrl(opcode_data.eValue<RVInstr>());
    };
    do_branch << [this] {
      return Control::do_branch_ctrl(opcode_exec.eValue<RVInstr>());
    };
    do_jump <<
        [this] { return Control::do_jump_ctrl(opcode_exec.eValue<RVInstr>()); };

    comp_ctrl <<
        [this] { return Control::do_comp_ctrl(opcode_exec.eValue<RVInstr>()); };
    mem_ctrl <<
        [this] { return Control::do_mem_ctrl(opcode_data.eValue<RVInstr>()); };

    exec_is_valid <<
        [this] { return isValidExec(opcode_exec.eValue<RVInstr>()); };
    data_is_valid <<
        [this] { return isValidData(opcode_data.eValue<RVInstr>()); };

    alu_exec_op1_ctrl << [this] {
      return Control::do_alu_op1_ctrl(opcode_exec.eValue<RVInstr>());
    };
    alu_exec_op2_ctrl << [this] {
      return Control::do_alu_op2_ctrl(opcode_exec.eValue<RVInstr>());
    };
    alu_exec_ctrl <<
        [this] { return Control::do_alu_ctrl(opcode_exec.eValue<RVInstr>()); };
  }

  INPUTPORT_ENUM(opcode_exec, RVInstr);
  INPUTPORT_ENUM(opcode_data, RVInstr);

  OUTPUTPORT_ENUM(reg_wr_src_exec_ctrl, RegWrExecSrc);

  OUTPUTPORT(reg_do_write_exec_ctrl, 1);
  OUTPUTPORT(reg_do_write_data_ctrl, 1);

  OUTPUTPORT(mem_do_write_ctrl, 1);
  OUTPUTPORT(do_branch, 1);
  OUTPUTPORT(do_jump, 1);
  OUTPUTPORT_ENUM(comp_ctrl, CompOp);
  OUTPUTPORT_ENUM(mem_ctrl, MemOp);

  OUTPUTPORT(exec_is_valid, 1);
  OUTPUTPORT(data_is_valid, 1);

  OUTPUTPORT_ENUM(alu_exec_op1_ctrl, AluSrc1);
  OUTPUTPORT_ENUM(alu_exec_op2_ctrl, AluSrc2);
  OUTPUTPORT_ENUM(alu_exec_ctrl, ALUOp);
};

} // namespace core
} // namespace vsrtl