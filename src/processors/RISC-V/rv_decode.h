#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class Decode : public Component {
public:
    void setISA(const std::shared_ptr<ISAInfoBase>& isa) { m_isa = isa; }

    Decode(const std::string& name, SimComponent* parent) : Component(name, parent) {
        opcode << [=] {
            const auto instrValue = instr.uValue();

            const unsigned l7 = instrValue & 0b1111111;

            // clang-format off
            switch(l7) {
            case RVISA::Opcode::LUI: return RVInstr::LUI;
            case RVISA::Opcode::AUIPC: return RVInstr::AUIPC;
            case RVISA::Opcode::JAL: return RVInstr::JAL;
            case RVISA::Opcode::JALR: return RVInstr::JALR;
            case RVISA::Opcode::ECALL: return RVInstr::ECALL;

            case RVISA::Opcode::OPIMM: {
                // I-Type
                const auto fields = RVInstrParser::getParser()->decodeI32Instr(instrValue);
                switch(fields[2]) {
                case 0b000: return RVInstr::ADDI;
                case 0b010: return RVInstr::SLTI;
                case 0b011: return RVInstr::SLTIU;
                case 0b100: return RVInstr::XORI;
                case 0b110: return RVInstr::ORI;
                case 0b111: return RVInstr::ANDI;
                case 0b001: return RVInstr::SLLI;
                case 0b101: {
                    switch (instrValue >> 26) {
                    case 0b000000: return RVInstr::SRLI;
                    case 0b010000: return RVInstr::SRAI;
                    }
                }
                default: break;
                }
                break;
            }

            case RVISA::Opcode::OPIMM32: {
                // I-Type (32-bit, in 64-bit ISA)
                const auto fields = RVInstrParser::getParser()->decodeI32Instr(instrValue);
                switch(fields[2]) {
                case 0b000: return RVInstr::ADDIW;
                case 0b001: return RVInstr::SLLIW;
                case 0b101: {
                    switch (instrValue >> 26) {
                    case 0b000000: return RVInstr::SRLIW;
                    case 0b010000: return RVInstr::SRAIW;
                    }
                }
                default: break;
                }
                break;
            }

            case RVISA::Opcode::OP: {
                // R-Type
                const auto fields = RVInstrParser::getParser()->decodeR32Instr(instrValue);
                if (fields[0] == 0b0000001) {
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
                                default: return RVInstr::NOP;
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
                                default: return RVInstr::NOP;
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

            case RVISA::Opcode::OP32: {
                // R-Type (32-bit, in 64-bit ISA)
                const auto fields = RVInstrParser::getParser()->decodeR32Instr(instrValue);
                if (fields[0] == 0b0000001) {
                    if(m_isa && m_isa->extensionEnabled("M")) {
                        // RV64M Standard extension
                        switch (fields[3]) {
                            case 0b000: return RVInstr::MULW;
                            case 0b100: return RVInstr::DIVW;
                            case 0b101: return RVInstr::DIVUW;
                            case 0b110: return RVInstr::REMW;
                            case 0b111: return RVInstr::REMUW;
                            default: break;
                        }
                    }
                } else {
                    switch (fields[3]) {
                        case 0b000: {
                            switch (fields[0]) {
                                case 0b0000000: return RVInstr::ADDW;
                                case 0b0100000: return RVInstr::SUBW;
                                default: return RVInstr::NOP;
                            }
                        }
                        case 0b001: return RVInstr::SLLW;
                        case 0b101: {
                            switch (fields[0]) {
                                case 0b0000000: return RVInstr::SRLW;
                                case 0b0100000: return RVInstr::SRAW;
                                default: return RVInstr::NOP;
                            }
                        }
                        default: break;
                    }
                    break;
                }
                break;
            }

            case RVISA::Opcode::LOAD: {
                // Load instruction
                const auto fields = RVInstrParser::getParser()->decodeI32Instr(instrValue);
                switch (fields[2]) {
                    case 0b000: return RVInstr::LB;
                    case 0b001: return RVInstr::LH;
                    case 0b010: return RVInstr::LW;
                    case 0b100: return RVInstr::LBU;
                    case 0b101: return RVInstr::LHU;
                    case 0b110: return RVInstr::LWU;
                    case 0b011: return RVInstr::LD;
                    default: break;
                }
                break;
            }

            case RVISA::Opcode::STORE: {
                // Store instructions
                const auto fields = RVInstrParser::getParser()->decodeS32Instr(instrValue);
                switch (fields[3]) {
                    case 0b000: return RVInstr::SB;
                    case 0b001: return RVInstr::SH;
                    case 0b010: return RVInstr::SW;
                    case 0b011: return RVInstr::SD;
                    default: break;
                }
                break;
            }

            case RVISA::Opcode::BRANCH: {
                // Branch instruction
                const auto fields = RVInstrParser::getParser()->decodeB32Instr(instrValue);
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

    INPUTPORT(instr, c_RVInstrWidth);
    OUTPUTPORT_ENUM(opcode, RVInstr);
    OUTPUTPORT(wr_reg_idx, c_RVRegsBits);
    OUTPUTPORT(r1_reg_idx, c_RVRegsBits);
    OUTPUTPORT(r2_reg_idx, c_RVRegsBits);

private:
    void unknownInstruction() {}
    std::shared_ptr<ISAInfoBase> m_isa;
};

}  // namespace core
}  // namespace vsrtl
