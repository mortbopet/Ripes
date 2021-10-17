#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

#define dprintf \
    if (0) {    \
    } else      \
        printf

// only support 32 bit instructions
VInt uncompress(const VInt instrValue, const Ripes::ISA isaID) {
    static VInt instr_last = 0;
    static VInt instr_cache = 0;

    // use cache to avoid run multiple times with same instruction in multiple calls
    if (instr_last == instrValue) {
        return instr_cache;
    }

    VInt new_instr = instrValue;
    long imm;
    unsigned uimm, rd, rs1, rs2;

    const int quadrant = instrValue & 0b11;
    const int func3 = (instrValue & 0xE000) >> 13;

    switch (quadrant) {
        case 0x00:  // quadrant
            switch (func3) {
                case 0b000: {  // c.addi4spn
                    const auto fields = RVInstrParser::getParser()->decodeCIW16Instr(instrValue);
                    rd = fields[3] | 0x8;
                    uimm = (((fields[2] & 0x3C) << 2) | ((fields[2] & 0xC0) >> 4) | ((fields[2] & 0x01) << 1) |
                            ((fields[2] & 0x02) >> 1))
                           << 2;
                    // addi rd ′ , x2, nzuimm[9:2]
                    new_instr = (uimm << 20) | (0b00010 << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                } break;
                // case 0b001: c.fld  RV32DC/RV64DC-only
                case 0b010: {  // c.lw
                    const auto fields = RVInstrParser::getParser()->decodeCS16Instr(instrValue);
                    rd = fields[5] | 0x8;
                    rs1 = fields[3] | 0x8;
                    uimm = ((fields[4] & 0x01) << 6) | (fields[2] << 3) | ((fields[4] & 0x02) << 1);
                    // lw rd ′ , offset[6:2](rs1 ′ )
                    new_instr = (uimm << 20) | (rs1 << 15) | (0b010 << 12) | (rd << 7) | RVISA::Opcode::LOAD;
                } break;
                case 0b011:
                    if (isaID == ISA::RV32I) {
                        // c.flw RV32FC-only
                    } else {
                        dprintf("TODO c.ld\n");
                    }
                    break;
                // case 0b100:  // RESERVED
                //    break;
                // case 0b101: c.fsd RV32DC/RV64DC-only
                case 0b110:  // c.sw
                {
                    const auto fields = RVInstrParser::getParser()->decodeCS16Instr(instrValue);
                    rs1 = fields[3] | 0x8;
                    rs2 = fields[5] | 0x8;
                    uimm = ((fields[4] & 0x01) << 6) | (fields[2] << 3) | ((fields[4] & 0x02) << 1);
                    // sw rs2 ′ ,offset[6:2](rs1 ′ )
                    new_instr = (((uimm & 0xFE0) >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (0b010 << 12) |
                                ((uimm & 0x1F) << 7) | RVISA::Opcode::STORE;
                } break;
                case 0b111:
                    if (isaID == ISA::RV32I) {
                        // c.fsw RV32FC-only
                    } else {
                        dprintf("TODO c.sd\n");
                    }
                    break;
            }
            break;
        case 0x01:  // quadrant
            switch (func3) {
                case 0b000:  // c.addi
                {
                    const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                    rd = fields[3];
                    imm = fields[4];
                    if (fields[2])
                        imm = -imm;
                    // addi rd, rd, nzimm[5:0]
                    new_instr = (imm << 20) | (rd << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                } break;
                case 0b001:
                    if (isaID == ISA::RV32I) {  // c.jal
                        const auto fields = RVInstrParser::getParser()->decodeCJ16Instr(instrValue);

                        imm = (fields[2] & 0x400) | ((fields[2] & 0x040) << 3) | (fields[2] & 0x180) |
                              ((fields[2] & 0x010) << 2) | (fields[2] & 0x020) | ((fields[2] & 0x001) << 5) |
                              ((fields[2] & 0x200) >> 6) | ((fields[2] & 0x00E) >> 1);
                        // jal x1,offset[11:1]
                        new_instr =
                            ((((imm & 0x7FF) << 10) | ((imm & 0x800) >> 3)) << 12) | (0b001 << 7) | RVISA::Opcode::JAL;
                    } else {
                        dprintf("TODO c.addiw\n");
                    }
                    break;
                case 0b010:  // C.LI
                {
                    const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                    // addi rd,x0, imm[5:0]
                    rd = fields[3];
                    imm = (fields[2] << 5) | fields[4];
                    if (imm & 0x20)  // test for negative
                    {
                        imm = imm | 0xFFFFFFC0;
                    }
                    new_instr = (imm << 20) | (rd << 7) | RVISA::Opcode::OPIMM;
                    break;
                }
                case 0b011: {
                    const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                    rd = fields[3];
                    if (rd == 2) {  // c.addi16sp
                        imm = (((fields[4] & 0x06) << 2) | ((fields[4] & 0x08) >> 1) | ((fields[4] & 0x01) << 1) |
                               ((fields[4] & 0x10) >> 4))
                              << 4;
                        if (fields[2]) {
                            imm = 0xFFE00 | imm;
                        }
                        // addi x2, x2,nzimm[9:4]
                        new_instr = (imm << 20) | (rd << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                    } else {  // c.lui
                        imm = fields[4];
                        if (fields[2]) {
                            imm = 0xFFFE0 | imm;
                        }
                        // lui rd, nzimm[17:12]
                        new_instr = (imm << 12) | (rd << 7) | RVISA::Opcode::LUI;
                    }
                } break;
                case 0b100:  // MISC-ALU
                {
                    const auto fields = RVInstrParser::getParser()->decodeCA16Instr(instrValue);
                    rd = fields[4] | 0x8;
                    rs2 = fields[6] | 0x8;
                    switch (fields[3]) {
                        case 0b00: {  // c.srli
                            const auto fieldscb = RVInstrParser::getParser()->decodeCB216Instr(instrValue);
                            uimm = (fieldscb[2] << 6) | fieldscb[5];
                            // srli rd ′ ,rd ′ , shamt[5:0]
                            new_instr = (uimm << 20) | (rd << 15) | (0b101 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                        } break;
                        case 0b01: {  // c.srai
                            const auto fieldscb = RVInstrParser::getParser()->decodeCB216Instr(instrValue);
                            uimm = (fieldscb[2] << 6) | fieldscb[5];
                            // srai rd ′ , rd ′ , shamt[5:0]
                            new_instr = (0b0100000 << 25) | (uimm << 20) | (rd << 15) | (0b101 << 12) | (rd << 7) |
                                        RVISA::Opcode::OPIMM;
                        } break;
                        case 0b10: {  // c.andi
                            const auto fieldscb = RVInstrParser::getParser()->decodeCB216Instr(instrValue);
                            imm = fieldscb[5];
                            if (fieldscb[2]) {
                                imm = 0xFE0 | imm;
                            }
                            // andi rd ′ ,rd ′ , imm[5:0]
                            new_instr = (imm << 20) | (rd << 15) | (0b111 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                        } break;
                        case 0b11:
                            switch (fields[2] << 2 | fields[5]) {
                                case 0b000:  // c.sub
                                    new_instr = (0b0100000 << 25) | (rs2 << 20) | (rd << 15) | (0b000 << 12) |
                                                (rd << 7) | RVISA::Opcode::OP;
                                    break;
                                case 0b001:  // c.xor
                                    new_instr =
                                        (rs2 << 20) | (rd << 15) | (0b100 << 12) | (rd << 7) | RVISA::Opcode::OP;
                                    break;
                                case 0b010:  // c.or
                                    new_instr =
                                        (rs2 << 20) | (rd << 15) | (0b110 << 12) | (rd << 7) | RVISA::Opcode::OP;
                                    break;
                                case 0b011:  // c.and
                                    new_instr =
                                        (rs2 << 20) | (rd << 15) | (0b111 << 12) | (rd << 7) | RVISA::Opcode::OP;
                                    break;
                                case 0b100:  // c.subw RV64C/RV128C-only
                                    new_instr = (0b0100000 << 25) | (rs2 << 20) | (rd << 15) | (0b000 << 12) |
                                                (rd << 7) | RVISA::Opcode::OP32;
                                    break;
                                case 0b101:  // c.addw RV64C/RV128C-only
                                    new_instr =
                                        (rs2 << 20) | (rd << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OP32;
                                    break;
                                    // case 0b110:  // RESERVED
                                    //    break;
                                    // case 0b111:  // RESERVED
                                    //    break;
                            }
                            break;
                    }
                    break;
                }
                case 0b101: {  // c.j
                    const auto fields = RVInstrParser::getParser()->decodeCJ16Instr(instrValue);

                    imm = (fields[2] & 0x400) | ((fields[2] & 0x040) << 3) | (fields[2] & 0x180) |
                          ((fields[2] & 0x010) << 2) | (fields[2] & 0x020) | ((fields[2] & 0x001) << 5) |
                          ((fields[2] & 0x200) >> 6) | ((fields[2] & 0x00E) >> 1);
                    // jal x0,offset[11:1]
                    new_instr =
                        ((((imm & 0x7FF) << 10) | ((imm & 0x800) >> 3)) << 12) | (0b000 << 7) | RVISA::Opcode::JAL;
                } break;
                case 0b110: {  // c.beqz
                    const auto fields = RVInstrParser::getParser()->decodeCB16Instr(instrValue);
                    rs1 = fields[3] | 0x8;
                    imm = ((fields[4] & 0x18) << 2) | ((fields[4] & 0x01) << 4) | ((fields[2] & 0x03) << 2) |
                          ((fields[4] & 0x06) >> 1);
                    if (fields[2] & 0x04) {
                        imm = 0xFF80 | imm;
                    }
                    // beq rs1 ′ , x0, offset[8:1]
                    new_instr = ((((imm & 0x8000) >> 9) | ((imm & 0x3F00) >> 8)) << 25) | (0b00 << 20) | (rs1 << 15) |
                                (0b000 << 12) | ((((imm & 0x000F) << 1) | ((imm & 0x4000) >> 14)) << 7) |
                                RVISA::Opcode::BRANCH;
                } break;
                case 0b111: {  // c.bnez
                    const auto fields = RVInstrParser::getParser()->decodeCB16Instr(instrValue);
                    rs1 = fields[3] | 0x8;
                    imm = ((fields[4] & 0x18) << 2) | ((fields[4] & 0x01) << 4) | ((fields[2] & 0x03) << 2) |
                          ((fields[4] & 0x06) >> 1);
                    if (fields[2] & 0x04) {
                        imm = 0xFF80 | imm;
                    }
                    // bne rs1 ′ , x0, offset[8:1]
                    new_instr = ((((imm & 0x8000) >> 9) | ((imm & 0x3F00) >> 8)) << 25) | (0b00 << 20) | (rs1 << 15) |
                                (0b001 << 12) | ((((imm & 0x000F) << 1) | ((imm & 0x4000) >> 14)) << 7) |
                                RVISA::Opcode::BRANCH;
                } break;
            }
            break;
        case 0x02:  // quadrant
            switch (func3) {
                case 0b000:  // c.slli
                {
                    const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                    if (!fields[2]) {
                        rd = fields[3];
                        uimm = fields[4];
                        // slli rd, rd, shamt[4:0]
                        new_instr = (uimm << 20) | (rd << 15) | (0b001 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                    }
                } break;
                // case 0b001: c.fldsp RV32DC/RV64DC-only
                case 0b010: {  // c.lwsp
                    const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                    rd = fields[3];
                    uimm = ((fields[4] & 0x03) << 7) | (fields[2] << 6) | (fields[4] & 0x1C);
                    // lw rd,offset[7:2](x2)
                    new_instr = (uimm << 20) | (0b0010 << 15) | (0b010 << 12) | (rd << 7) | RVISA::Opcode::LOAD;
                } break;
                case 0b011:
                    if (isaID == ISA::RV32I) {
                        // c.flwsp RV32FC-only
                    } else {
                        dprintf("TODO c.ldsp\n");
                    }
                    break;
                case 0b100: {
                    const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                    rd = fields[3];
                    rs2 = fields[4];
                    if (fields[2]) {
                        if (rs2) {  // c.add
                            // add rd, rd, rs2
                            new_instr = (rs2 << 20) | (rd << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OP;
                        } else {
                            if (rd) {  // c.jarl
                                // jalr x1, 0(rs1)
                                new_instr =
                                    (0b0 << 20) | (rd << 15) | (0b000 << 12) | (0b00001 << 7) | RVISA::Opcode::JALR;
                            }
                            // else{
                            // c.ebreak  -> ebreak  Not implemented in Ripes
                            //}
                        }
                    } else {
                        if (rs2) {  // c.mv
                                    // add rd, x0, rs2
                            new_instr = (rs2 << 20) | (0b0 << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OP;
                        } else {  // c.jr
                            // jalr x0, 0(rs1)
                            new_instr = (0b0 << 20) | (rd << 15) | (0b000 << 12) | (0b00000 << 7) | RVISA::Opcode::JALR;
                        }
                    }
                } break;
                // case 0b101: c.fsdsp RV32DC/RV64DC-only
                case 0b110:  // c.swsp
                {
                    const auto fields = RVInstrParser::getParser()->decodeCSS16Instr(instrValue);
                    rs2 = fields[3];
                    uimm = ((fields[2] & 0x03) << 6) | (fields[2] & 0x3C);
                    // sw rs2,offset[7:2](x2)
                    new_instr = (((uimm & 0xFE0) >> 5) << 25) | (rs2 << 20) | (0b00010 << 15) | (0b010 << 12) |
                                ((uimm & 0x1F) << 7) | RVISA::Opcode::STORE;
                } break;
                case 0b111:
                    if (isaID == ISA::RV32I) {
                        // c.fswsp RV32FC-only
                    } else {
                        dprintf("TODO c.sdsp\n");
                    }
                    break;
            }
            break;
        default:  // No compressed
            break;
    }

    instr_last = instrValue;
    instr_cache = new_instr;

    return new_instr;
}

template <unsigned XLEN>
class Decode : public Component {
public:
    void setISA(const std::shared_ptr<ISAInfoBase>& isa) { m_isa = isa; }

    Decode(std::string name, SimComponent* parent) : Component(name, parent) {
        opcode << [=] {
            const auto instrValue = uncompress(instr.uValue(), m_isa->isaID());

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
          return (uncompress(instr.uValue(), m_isa->isaID())>> 7) & 0b11111;
        };

        r1_reg_idx << [=] {
          return (uncompress(instr.uValue(), m_isa->isaID())>> 15) & 0b11111;
        };

        r2_reg_idx << [=] {
          return (uncompress(instr.uValue(), m_isa->isaID())>> 20) & 0b11111;
        };

        Pc_Inc << [=] {
            return (instr.uValue() & 0b11) == 0b11;
        };

        exp_instr << [=] {
            return uncompress(instr.uValue(), m_isa->isaID());
        };

        // clang-format on
    }

    INPUTPORT(instr, c_RVInstrWidth);
    OUTPUTPORT_ENUM(opcode, RVInstr);
    OUTPUTPORT(wr_reg_idx, c_RVRegsBits);
    OUTPUTPORT(r1_reg_idx, c_RVRegsBits);
    OUTPUTPORT(r2_reg_idx, c_RVRegsBits);
    OUTPUTPORT(Pc_Inc, 1);
    OUTPUTPORT(exp_instr, c_RVInstrWidth);

private:
    void unknownInstruction() {}
    std::shared_ptr<ISAInfoBase> m_isa;
};

}  // namespace core
}  // namespace vsrtl
