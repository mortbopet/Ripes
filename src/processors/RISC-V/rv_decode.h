#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class Decode : public Component {
public:
    void setISA(const std::shared_ptr<ISAInfo<ISA::RV32I>>& isa) { m_isa = isa; }

    Decode(std::string name, SimComponent* parent) : Component(name, parent) {
        opcode << [=] {
            const unsigned l7 = instr.uValue() & 0b1111111;

            // clang-format off
            switch(l7) {
            case 0b0110111: return RVInstr::LUI;
            case 0b0010111: return RVInstr::AUIPC;
            case 0b1101111: return RVInstr::JAL;
            case 0b1100111: return RVInstr::JALR;
            case 0b1110011: return RVInstr::ECALL;

            case 0b0010011: {
                // I-Type
                const auto fields = RVInstrParser::getParser()->decodeIInstr(instr.uValue());
                switch(fields[2]) {
                case 0b000: return RVInstr::ADDI;
                case 0b010: return RVInstr::SLTI;
                case 0b011: return RVInstr::SLTIU;
                case 0b100: return RVInstr::XORI;
                case 0b110: return RVInstr::ORI;
                case 0b111: return RVInstr::ANDI;
                case 0b001: return RVInstr::SLLI;
                case 0b101: {
                    switch (instr.uValue() >> 25) {
                    case 0b0: return RVInstr::SRLI;
                    case 0b0100000: return RVInstr::SRAI;
                    }
                }
                default: break;
                }
                break;
            }

            case 0b0110011: {
                // R-Type
                const auto fields = RVInstrParser::getParser()->decodeRInstr(instr.uValue());
                if (fields[0] == 0b1) {
                    if(m_isa && m_isa->extensionEnabled("M")) {
                        // RV32M Standard extension
                        switch (fields[3]) {
                            case 0b000: return RVInstr::MUL;
                            case 0b001: return RVInstr::MULH;
                            case 0b010: return RVInstr::MULHSU;
                            case 0b011: return RVInstr::MULHU;
                            case 0b100: return RVInstr::DIV;
                            case 0b101: return RVInstr::DIVU;
                            case 0b110: return RVInstr::REM;
                            case 0b111: return RVInstr::REMU;
                            default: break;
                        }
                    }
                } else {
                    switch (fields[3]) {
                        case 0b000: {
                            switch (fields[0]) {
                                case 0b0000000: return RVInstr::ADD;
                                case 0b0100000: return RVInstr::SUB;
                            }
                        }
                        case 0b001: return RVInstr::SLL;
                        case 0b010: return RVInstr::SLT;
                        case 0b011: return RVInstr::SLTU;
                        case 0b100: return RVInstr::XOR;
                        case 0b101: {
                            switch (fields[0]) {
                                case 0b0000000: return RVInstr::SRL;
                                case 0b0100000: return RVInstr::SRA;
                            }
                        }
                        case 0b110: return RVInstr::OR;
                        case 0b111: return RVInstr::AND;
                        default: break;
                    }
                    break;
                }
                break;

            }

            case 0b0000011: {
                // Load instruction
                const auto fields = RVInstrParser::getParser()->decodeIInstr(instr.uValue());
                switch (fields[2]) {
                    case 0b000: return RVInstr::LB;
                    case 0b001: return RVInstr::LH;
                    case 0b010: return RVInstr::LW;
                    case 0b100: return RVInstr::LBU;
                    case 0b101: return RVInstr::LHU;
                    default: break;
                }
                break;
            }

            case 0b0100011: {
                // Store instructions
                const auto fields = RVInstrParser::getParser()->decodeSInstr(instr.uValue());
                switch (fields[3]) {
                    case 0b000: return RVInstr::SB;
                    case 0b001: return RVInstr::SH;
                    case 0b010: return RVInstr::SW;
                    default: break;
                }
                break;
            }

            case 0b1100011: {
                // Branch instruction
                const auto fields = RVInstrParser::getParser()->decodeBInstr(instr.uValue());
                switch (fields[4]) {
                    case 0b000: return RVInstr::BEQ;
                    case 0b001: return RVInstr::BNE;
                    case 0b100: return RVInstr::BLT;
                    case 0b101: return RVInstr::BGE;
                    case 0b110: return RVInstr::BLTU;
                    case 0b111: return RVInstr::BGEU;
                    default: break;
                }
                break;
            }


            default:
                break;
            }

            // Fallthrough - unknown instruction.
            return RVInstr::NOP;
        };

        wr_reg_idx << [=] {
            return (instr.uValue() >> 7) & 0b11111;
        };

        r1_reg_idx << [=] {
            return (instr.uValue() >> 15) & 0b11111;
        };

        r2_reg_idx << [=] {
            return (instr.uValue() >> 20) & 0b11111;
        };

        // clang-format on
    }

    INPUTPORT(instr, RV_INSTR_WIDTH);
    OUTPUTPORT_ENUM(opcode, RVInstr);
    OUTPUTPORT(wr_reg_idx, RV_REGS_BITS);
    OUTPUTPORT(r1_reg_idx, RV_REGS_BITS);
    OUTPUTPORT(r2_reg_idx, RV_REGS_BITS);

private:
    void unknownInstruction() {}
    std::shared_ptr<ISAInfo<ISA::RV32I>> m_isa;
};

}  // namespace core
}  // namespace vsrtl
