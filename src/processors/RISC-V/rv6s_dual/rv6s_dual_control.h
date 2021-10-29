#pragma once

#include "../rv_control.h"
#include "VSRTL/core/vsrtl_component.h"
#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class Control_DUAL : public Component {
    static VSRTL_VT_U do_reg_wr_src_ctrl_dual(const VSRTL_VT_U& opc) {
        switch (opc) {
            // Jump instructions
            case RVInstr::JALR:
            case RVInstr::JAL:
                return RegWrSrcDual::PC4;

            default:
                return RegWrSrcDual::ALURES;
        }
    }

    static VSRTL_VT_U do_reg_wr_src_ctrl_data(const VSRTL_VT_U& opc) {
        if (Control::do_mem_ctrl(opc) != +MemOp::NOP) {
            return RegWrSrcDataDual::MEM;
        } else {
            return RegWrSrcDataDual::ALURES;
        }
    }

public:
    Control_DUAL(const std::string& name, SimComponent* parent) : Component(name, parent) {
        comp_ctrl << [=] { return exec_valid.uValue() ? Control::do_comp_ctrl(opcode_exec.uValue()) : +CompOp::NOP; };
        do_branch << [=] { return exec_valid.uValue() ? Control::do_branch_ctrl(opcode_exec.uValue()) : 0; };
        do_jump << [=] { return exec_valid.uValue() ? Control::do_jump_ctrl(opcode_exec.uValue()) : 0; };
        mem_ctrl << [=] { return data_valid.uValue() ? Control::do_mem_ctrl(opcode_data.uValue()) : +MemOp::NOP; };

        reg_do_write_ctrl_exec <<
            [=] { return exec_valid.uValue() && Control::do_reg_do_write_ctrl(opcode_exec.uValue()); };
        reg_do_write_ctrl_data <<
            [=] { return data_valid.uValue() && Control::do_reg_do_write_ctrl(opcode_data.uValue()); };

        reg_wr_src_ctrl << [=] { return do_reg_wr_src_ctrl_dual(opcode_exec.uValue()); };
        reg_wr_src_data_ctrl << [=] { return do_reg_wr_src_ctrl_data(opcode_data.uValue()); };

        alu_op1_ctrl_exec <<
            [=] { return exec_valid.uValue() ? Control::do_alu_op1_ctrl(opcode_exec.uValue()) : +AluSrc1::REG1; };
        alu_op2_ctrl_exec <<
            [=] { return exec_valid.uValue() ? Control::do_alu_op2_ctrl(opcode_exec.uValue()) : +AluSrc2::REG2; };
        alu_ctrl_exec << [=] { return exec_valid.uValue() ? Control::do_alu_ctrl(opcode_exec.uValue()) : +ALUOp::NOP; };

        alu_op2_ctrl_data <<
            [=] { return data_valid.uValue() ? Control::do_alu_op2_ctrl(opcode_data.uValue()) : +AluSrc2::REG2; };
        alu_ctrl_data << [=] { return data_valid.uValue() ? Control::do_alu_ctrl(opcode_data.uValue()) : +ALUOp::NOP; };

        mem_do_write_ctrl << [=] { return Control::do_do_mem_write_ctrl(opcode_data.uValue()); };
        mem_do_read_ctrl << [=] { return Control::do_do_read_ctrl(opcode_data.uValue()); };
    }

    INPUTPORT_ENUM(opcode_exec, RVInstr);
    INPUTPORT_ENUM(opcode_data, RVInstr);

    INPUTPORT(exec_valid, 1);
    INPUTPORT(data_valid, 1);

    OUTPUTPORT(reg_do_write_ctrl_exec, 1);
    OUTPUTPORT_ENUM(reg_wr_src_ctrl, RegWrSrcDual);
    OUTPUTPORT_ENUM(reg_wr_src_data_ctrl, RegWrSrcDataDual);

    OUTPUTPORT(reg_do_write_ctrl_data, 1);

    OUTPUTPORT(mem_do_write_ctrl, 1);
    OUTPUTPORT(mem_do_read_ctrl, 1);
    OUTPUTPORT(do_branch, 1);
    OUTPUTPORT(do_jump, 1);
    OUTPUTPORT_ENUM(comp_ctrl, CompOp);
    OUTPUTPORT_ENUM(mem_ctrl, MemOp);

    OUTPUTPORT_ENUM(alu_op1_ctrl_exec, AluSrc1);
    OUTPUTPORT_ENUM(alu_op2_ctrl_exec, AluSrc2);
    OUTPUTPORT_ENUM(alu_ctrl_exec, ALUOp);

    OUTPUTPORT_ENUM(alu_op2_ctrl_data, AluSrc2);
    OUTPUTPORT_ENUM(alu_ctrl_data, ALUOp);
};

}  // namespace core
}  // namespace vsrtl
