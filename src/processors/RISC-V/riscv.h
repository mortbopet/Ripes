#pragma once

#include <functional>
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

/**
 * @brief The FinishingCounter struct
 * May be used to control how many cycles are allowed to be performed once a processor has been designated to begin its
 * finishing sequence. Increment/decrement operators may be used in conjunction with clocking/reversing the circuit.
 */
struct FinishingCounter {
    void start(int _target) {
        if (finishing || finished)
            return;  // Already running
        target = _target;
        finishing = true;
        count = 0;
        finished = false;
    }
    void reset() {
        finishing = false;
        finished = false;
    }

    bool finishing = false;
    int count;
    int target;
    bool finished = false;
    void operator++(int) {
        if (!finishing || finished)
            return;
        count++;
        if (count == target) {
            finishing = false;
            finished = true;
        }
    }
    void operator--(int) {
        if (finished) {
            finished = false;
            finishing = true;
        }
        if (!finishing)
            return;
        count--;
        if (count == 0)
            finishing = false;
    }
};

}  // namespace Ripes
