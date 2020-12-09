#pragma once

#include <functional>
#include "../../isa/rv32isainfo.h"
#include "VSRTL/core/vsrtl_enum.h"
#include "VSRTL/interface/vsrtl.h"
#include "VSRTL/interface/vsrtl_binutils.h"
#include "rv_instrparser.h"

namespace Ripes {

#define RV_INSTR_WIDTH 32
#define RV_REG_WIDTH 32

#define RV_REGS 32
#define RV_REGS_BITS ceillog2(RV_REGS)

/** Instruction set enumerations */
Enum(RVInstrType, R, I, S, B, U, J);

Enum(RVInstr, NOP,
     /* RV32I Base Instruction Set */
     LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU, LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI, SLTIU, XORI,
     ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, ECALL,

     /* RV32M Standard Extension */
     MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU);

/** Datapath enumerations */
Enum(ALUOp, NOP, ADD, SUB, MUL, DIV, AND, OR, XOR, SL, SRA, SRL, LUI, LT, LTU, EQ, MULH, MULHU, MULHSU, DIVU, REM,
     REMU);
Enum(RegWrSrc, MEMREAD, ALURES, PC4);
Enum(AluSrc1, REG1, PC);
Enum(AluSrc2, REG2, IMM);
Enum(CompOp, NOP, EQ, NE, LT, LTU, GE, GEU);
Enum(MemOp, NOP, LB, LH, LW, LBU, LHU, SB, SH, SW);
Enum(ECALL, none, print_int = 1, print_char = 2, print_string = 4, exit = 10);
Enum(PcSrc, PC4 = 0, ALU = 1);

/** Instruction field parser */
class RVInstrParser {
public:
    static RVInstrParser* getParser() {
        static RVInstrParser parser;
        return &parser;
    }

    std::vector<::vsrtl::VSRTL_VT_U> decodeUInstr(::vsrtl::VSRTL_VT_U instr) const { return m_decodeUInstr(instr); }
    std::vector<::vsrtl::VSRTL_VT_U> decodeJInstr(::vsrtl::VSRTL_VT_U instr) const { return m_decodeJInstr(instr); }
    std::vector<::vsrtl::VSRTL_VT_U> decodeIInstr(::vsrtl::VSRTL_VT_U instr) const { return m_decodeIInstr(instr); }
    std::vector<::vsrtl::VSRTL_VT_U> decodeSInstr(::vsrtl::VSRTL_VT_U instr) const { return m_decodeSInstr(instr); }
    std::vector<::vsrtl::VSRTL_VT_U> decodeRInstr(::vsrtl::VSRTL_VT_U instr) const { return m_decodeRInstr(instr); }
    std::vector<::vsrtl::VSRTL_VT_U> decodeBInstr(::vsrtl::VSRTL_VT_U instr) const { return m_decodeBInstr(instr); }

private:
    RVInstrParser() {
        m_decodeRInstr = generateInstrParser(std::vector<int>{5, 3, 5, 5, 7});  // from LSB to MSB
        m_decodeIInstr = generateInstrParser(std::vector<int>{5, 3, 5, 12});
        m_decodeSInstr = generateInstrParser(std::vector<int>{5, 3, 5, 5, 7});
        m_decodeBInstr = generateInstrParser(std::vector<int>{1, 4, 3, 5, 5, 6, 1});
        m_decodeUInstr = generateInstrParser(std::vector<int>{5, 20});
        m_decodeJInstr = generateInstrParser(std::vector<int>{5, 8, 1, 10, 1});
    }
    decode_functor m_decodeUInstr;
    decode_functor m_decodeJInstr;
    decode_functor m_decodeIInstr;
    decode_functor m_decodeSInstr;
    decode_functor m_decodeRInstr;
    decode_functor m_decodeBInstr;
};

}  // namespace Ripes
