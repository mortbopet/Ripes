#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class Control : public Component {
public:
    /* clang-format off */
    static VSRTL_VT_U do_comp_ctrl(const VSRTL_VT_U& opc) {
        switch(opc){
            case RVInstr::BEQ: return CompOp::EQ;
            case RVInstr::BNE: return CompOp::NE;
            case RVInstr::BLT: return CompOp::LT;
            case RVInstr::BGE: return CompOp::GE;
            case RVInstr::BLTU: return CompOp::LTU;
            case RVInstr::BGEU: return CompOp::GEU;
            default: return CompOp::NOP;
        }
    }

    static VSRTL_VT_U do_branch_ctrl(const VSRTL_VT_U& opc) {
        switch(opc){
            case RVInstr::BEQ: case RVInstr::BNE: case RVInstr::BLT:
            case RVInstr::BGE: case RVInstr::BLTU: case RVInstr::BGEU:
                return 1;
            default:
                return 0;
        }
    }

    static VSRTL_VT_U do_jump_ctrl(const VSRTL_VT_U& opc) {
        switch(opc){
            case RVInstr::JAL: case RVInstr::JALR:
                return 1;
            default:
                return 0;
        }
    }

    static VSRTL_VT_U do_mem_ctrl(const VSRTL_VT_U& opc) {
        switch(opc){
            case RVInstr::SB: return MemOp::SB;
            case RVInstr::SH: return MemOp::SH;
            case RVInstr::SW: return MemOp::SW;
            case RVInstr::LB: return MemOp::LB;
            case RVInstr::LH: return MemOp::LH;
            case RVInstr::LW: return MemOp::LW;
            case RVInstr::LBU: return MemOp::LBU;
            case RVInstr::LHU: return MemOp::LHU;
            default:
                return MemOp::NOP;
        }
    }

    static VSRTL_VT_U do_reg_do_write_ctrl(const VSRTL_VT_U& opc) {
        switch(opc) {
            case RVInstr::LUI:
            case RVInstr::AUIPC:

            // Arithmetic-immediate instructions
            case RVInstr::ADDI: case RVInstr::SLTI: case RVInstr::SLTIU: case RVInstr::XORI:
            case RVInstr::ORI: case RVInstr::ANDI: case RVInstr::SLLI: case RVInstr::SRLI:
            case RVInstr::SRAI:

            // Arithmetic instructions
            case RVInstr::MUL: case RVInstr::MULH: case RVInstr:: MULHSU: case RVInstr::MULHU:
            case RVInstr::DIV: case RVInstr::DIVU: case RVInstr::REM: case RVInstr::REMU:
            case RVInstr::ADD: case RVInstr::SUB: case RVInstr::SLL: case RVInstr::SLT:
            case RVInstr::SLTU: case RVInstr::XOR: case RVInstr::SRL: case RVInstr::SRA:
            case RVInstr::OR: case RVInstr::AND:

            // Load instructions
            case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU:

            // Jump instructions
            case RVInstr::JALR:
            case RVInstr::JAL:
                return 1;
            default: return 0;
        }
    }

    static VSRTL_VT_U do_reg_wr_src_ctrl(const VSRTL_VT_U& opc) {
        switch(opc){
            // Load instructions
            case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU:
                return RegWrSrc::MEMREAD;

            // Jump instructions
            case RVInstr::JALR:
            case RVInstr::JAL:
                return RegWrSrc::PC4;

            default:
                return RegWrSrc::ALURES;
        }
    }

    static VSRTL_VT_U do_alu_op1_ctrl(const VSRTL_VT_U& opc) {
        switch(opc) {
            case RVInstr::AUIPC: case RVInstr::JAL:
            case RVInstr::BEQ: case RVInstr::BNE: case RVInstr::BLT:
            case RVInstr::BGE: case RVInstr::BLTU: case RVInstr::BGEU:
                return AluSrc1::PC;
            default:
                return AluSrc1::REG1;
        }
    }

    static VSRTL_VT_U do_alu_op2_ctrl(const VSRTL_VT_U& opc) {
        switch(opc) {
        case RVInstr::LUI:
        case RVInstr::AUIPC:
            return AluSrc2::IMM;

        // Arithmetic-immediate instructions
        case RVInstr::ADDI: case RVInstr::SLTI: case RVInstr::SLTIU: case RVInstr::XORI:
        case RVInstr::ORI: case RVInstr::ANDI: case RVInstr::SLLI: case RVInstr::SRLI:
        case RVInstr::SRAI:
            return AluSrc2::IMM;

        // Arithmetic instructions
        case RVInstr::MUL: case RVInstr::MULH: case RVInstr:: MULHSU: case RVInstr::MULHU:
        case RVInstr::DIV: case RVInstr::DIVU: case RVInstr::REM: case RVInstr::REMU:
        case RVInstr::ADD: case RVInstr::SUB: case RVInstr::SLL: case RVInstr::SLT:
        case RVInstr::SLTU: case RVInstr::XOR: case RVInstr::SRL: case RVInstr::SRA:
        case RVInstr::OR: case RVInstr::AND:
            return AluSrc2::REG2;

        // Load/Store instructions
        case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU:
        case RVInstr::SB: case RVInstr::SH: case RVInstr::SW:
            return AluSrc2::IMM;

        // Branch instructions
        case RVInstr::BEQ: case RVInstr::BNE: case RVInstr::BLT:
        case RVInstr::BGE: case RVInstr::BLTU: case RVInstr::BGEU:
            return AluSrc2::IMM;

        // Jump instructions
        case RVInstr::JALR:
        case RVInstr::JAL:
            return AluSrc2::IMM;

        default:
            return AluSrc2::REG2;
        }
    }

    static VSRTL_VT_U do_alu_ctrl(const VSRTL_VT_U& opc) {
        switch(opc) {
            case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU:
            case RVInstr::SB: case RVInstr::SH: case RVInstr::SW:
                return ALUOp::ADD;
            case RVInstr::LUI:
                return ALUOp::LUI;
            case RVInstr::JAL: case RVInstr::JALR: case RVInstr::AUIPC:
            case RVInstr::ADD: case RVInstr::ADDI:
            case RVInstr::BEQ: case RVInstr::BNE: case RVInstr::BLT:
            case RVInstr::BGE: case RVInstr::BLTU: case RVInstr::BGEU:
                return ALUOp::ADD;
            case RVInstr::SUB:
                return ALUOp::SUB;
            case RVInstr::SLT: case RVInstr::SLTI:
                return ALUOp::LT;
            case RVInstr::SLTU: case RVInstr::SLTIU:
                return ALUOp::LTU;
            case RVInstr::XOR: case RVInstr::XORI:
                return ALUOp::XOR;
            case RVInstr::OR: case RVInstr::ORI:
                return ALUOp::OR;
            case RVInstr::AND: case RVInstr::ANDI:
                return ALUOp::AND;
            case RVInstr::SLL: case RVInstr::SLLI:
                return ALUOp::SL;
            case RVInstr::SRL: case RVInstr::SRLI:
                return ALUOp::SRL;
            case RVInstr::SRA: case RVInstr::SRAI:
                return ALUOp::SRA;
            case RVInstr::MUL:
                return ALUOp::MUL;
            case RVInstr::MULH:
                return ALUOp::MULH;
            case RVInstr::MULHU:
                return ALUOp::MULHU;
            case RVInstr::MULHSU:
                return ALUOp::MULHSU;
            case RVInstr::DIV:
                return ALUOp::DIV;
            case RVInstr::DIVU:
                return ALUOp::DIVU;
            case RVInstr::REM:
                return ALUOp::REM;
            case RVInstr::REMU:
                return ALUOp::REMU;
            default: return ALUOp::NOP;
        }
    }

    static VSRTL_VT_U do_do_mem_write_ctrl(const VSRTL_VT_U& opc) {
        switch(opc) {
            case RVInstr::SB: case RVInstr::SH: case RVInstr::SW:
                return 1;
            default: return 0;
        }
    }

    static VSRTL_VT_U do_do_read_ctrl(const VSRTL_VT_U& opc) {
        switch(opc) {
            case RVInstr::LB: case RVInstr::LH: case RVInstr::LW: case RVInstr::LBU: case RVInstr::LHU:
                return 1;
            default: return 0;
        }
    }
    /* clang-format on */

public:
    Control(std::string name, SimComponent* parent) : Component(name, parent) {
        comp_ctrl << [=] { return do_comp_ctrl(opcode.uValue()); };
        do_branch << [=] { return do_branch_ctrl(opcode.uValue()); };
        do_jump << [=] { return do_jump_ctrl(opcode.uValue()); };
        mem_ctrl << [=] { return do_mem_ctrl(opcode.uValue()); };
        reg_do_write_ctrl << [=] { return do_reg_do_write_ctrl(opcode.uValue()); };
        reg_wr_src_ctrl << [=] { return do_reg_wr_src_ctrl(opcode.uValue()); };
        alu_op1_ctrl << [=] { return do_alu_op1_ctrl(opcode.uValue()); };
        alu_op2_ctrl << [=] { return do_alu_op2_ctrl(opcode.uValue()); };
        alu_ctrl << [=] { return do_alu_ctrl(opcode.uValue()); };
        mem_do_write_ctrl << [=] { return do_do_mem_write_ctrl(opcode.uValue()); };
        mem_do_read_ctrl << [=] { return do_do_read_ctrl(opcode.uValue()); };
    }

    INPUTPORT_ENUM(opcode, RVInstr);

    OUTPUTPORT(reg_do_write_ctrl, 1);
    OUTPUTPORT(mem_do_write_ctrl, 1);
    OUTPUTPORT(mem_do_read_ctrl, 1);
    OUTPUTPORT(do_branch, 1);
    OUTPUTPORT(do_jump, 1);
    OUTPUTPORT_ENUM(comp_ctrl, CompOp);
    OUTPUTPORT_ENUM(reg_wr_src_ctrl, RegWrSrc);
    OUTPUTPORT_ENUM(mem_ctrl, MemOp);
    OUTPUTPORT_ENUM(alu_op1_ctrl, AluSrc1);
    OUTPUTPORT_ENUM(alu_op2_ctrl, AluSrc2);
    OUTPUTPORT_ENUM(alu_ctrl, ALUOp);
};

}  // namespace core
}  // namespace vsrtl
