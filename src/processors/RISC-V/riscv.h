#pragma once

#include <functional>

#include "VSRTL/core/vsrtl_enum.h"
#include "VSRTL/interface/vsrtl_binutils.h"
#include "VSRTL/interface/vsrtl_interface.h"

#include "rv_instrparser.h"

#include "../../isa/rv32isainfo.h"
#include "../../isa/rv64isainfo.h"
#include "../../isa/rvisainfo_common.h"

namespace Ripes {

constexpr int c_RVInstrWidth = 32;                // Width of instructions
constexpr int c_RVRegs = 32;                      // Number of registers
constexpr int c_RVRegsBits = ceillog2(c_RVRegs);  // Width of operand to index into registers

template <unsigned XLEN>
constexpr Ripes::ISA XLenToRVISA() {
    static_assert(XLEN == 32 || XLEN == 64, "Only supports 32- and 64-bit variants");
    static_assert(vsrtl::VSRTL_VT_BITS >= XLEN, "Register size larger than VSRTL base type size");
    if (XLEN == 32) {
        return ISA::RV32I;
    } else {
        return ISA::RV64I;
    }
}

/** Instruction set enumerations */
Enum(RVInstrType, R, I, S, B, U, J);

Enum(RVInstr, NOP,
     /* RV32I Base Instruction Set */
     LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU, LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI, SLTIU, XORI,
     ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, ECALL,

     /* RV32M Standard Extension */
     MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU,

     /* RV64I Base Instruction Set */
     ADDIW, SLLIW, SRLIW, SRAIW, ADDW, SUBW, SLLW, SRLW, SRAW, LWU, LD, SD,

     /* RV64M Standard Extension */
     MULW, DIVW, DIVUW, REMW, REMUW);

/** Datapath enumerations */
Enum(ALUOp, NOP, ADD, SUB, MUL, DIV, AND, OR, XOR, SL, SRA, SRL, LUI, LT, LTU, EQ, MULH, MULHU, MULHSU, DIVU, REM, REMU,
     SLW, SRLW, SRAW, ADDW, SUBW, MULW, DIVW, DIVUW, REMW, REMUW);
Enum(RegWrSrc, MEMREAD, ALURES, PC4);
Enum(AluSrc1, REG1, PC);
Enum(AluSrc2, REG2, IMM);
Enum(CompOp, NOP, EQ, NE, LT, LTU, GE, GEU);
Enum(MemOp, NOP, LB, LH, LW, LBU, LHU, SB, SH, SW, LWU, LD, SD);
Enum(ECALL, none, print_int = 1, print_char = 2, print_string = 4, exit = 10);
Enum(PcSrc, PC4 = 0, ALU = 1);

/** Instruction field parser */
class RVInstrParser {
public:
    static RVInstrParser* getParser() {
        static RVInstrParser parser;
        return &parser;
    }

    std::vector<uint32_t> decodeU32Instr(const uint32_t& instr) const { return m_decodeU32Instr(instr); }
    std::vector<uint32_t> decodeJ32Instr(const uint32_t& instr) const { return m_decodeJ32Instr(instr); }
    std::vector<uint32_t> decodeI32Instr(const uint32_t& instr) const { return m_decodeI32Instr(instr); }
    std::vector<uint32_t> decodeS32Instr(const uint32_t& instr) const { return m_decodeS32Instr(instr); }
    std::vector<uint32_t> decodeR32Instr(const uint32_t& instr) const { return m_decodeR32Instr(instr); }
    std::vector<uint32_t> decodeB32Instr(const uint32_t& instr) const { return m_decodeB32Instr(instr); }

private:
    RVInstrParser() {
        m_decodeR32Instr = generateInstrParser<uint32_t>(std::vector<int>{7, 5, 3, 5, 5, 7});  // from LSB to MSB
        m_decodeI32Instr = generateInstrParser<uint32_t>(std::vector<int>{7, 5, 3, 5, 12});
        m_decodeS32Instr = generateInstrParser<uint32_t>(std::vector<int>{7, 5, 3, 5, 5, 7});
        m_decodeB32Instr = generateInstrParser<uint32_t>(std::vector<int>{7, 1, 4, 3, 5, 5, 6, 1});
        m_decodeU32Instr = generateInstrParser<uint32_t>(std::vector<int>{7, 5, 20});
        m_decodeJ32Instr = generateInstrParser<uint32_t>(std::vector<int>{7, 5, 8, 1, 10, 1});
    }
    decode_functor<uint32_t> m_decodeU32Instr;
    decode_functor<uint32_t> m_decodeJ32Instr;
    decode_functor<uint32_t> m_decodeI32Instr;
    decode_functor<uint32_t> m_decodeS32Instr;
    decode_functor<uint32_t> m_decodeR32Instr;
    decode_functor<uint32_t> m_decodeB32Instr;
};

}  // namespace Ripes
