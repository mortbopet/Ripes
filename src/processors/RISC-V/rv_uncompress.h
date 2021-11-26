#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class Uncompress : public Component {
public:
    void setISA(const std::shared_ptr<ISAInfoBase>& isa) {
        m_isa = isa;
        m_disabled = !m_isa->extensionEnabled("C");
    }

    Uncompress(std::string name, SimComponent* parent) : Component(name, parent) {
        setDescription("Uncompresses instructions from the 'C' extension into their 32-bit representation.");
        Pc_Inc << [=] {
            if (m_disabled)
                return true;
            return (((instr.uValue() & 0b11) == 0b11) || (!instr.uValue()));
        };

        // only support 32 bit instructions
        exp_instr << [=] {
            const auto instrValue = instr.uValue();
            const int quadrant = instrValue & 0b11;

            if ((quadrant == 0b11) || (m_disabled)) {  // Not a compressed instruction
                return instrValue;
            }

            VInt new_instr = instrValue;
            long imm;
            unsigned uimm, rd, rs1, rs2;

            const int func3 = (instrValue & 0xE000) >> 13;

            switch (quadrant) {
                case 0x00:  // quadrant
                    switch (func3) {
                        case 0b000: {          // c.addi4spn
                            if (instrValue) {  // not illegal instruction
                                const auto fields = RVInstrParser::getParser()->decodeCIW16Instr(instrValue);
                                rd = fields[3] | 0x8;
                                uimm = (((fields[2] & 0x3C) << 2) | ((fields[2] & 0xC0) >> 4) |
                                        ((fields[2] & 0x01) << 1) | ((fields[2] & 0x02) >> 1))
                                       << 2;
                                // addi rd ′ , x2, nzuimm[9:2]
                                new_instr =
                                    (uimm << 20) | (0b00010 << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                            }
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
                            if (m_isa->isaID() == ISA::RV64I) {  // c.ld
                                const auto fields = RVInstrParser::getParser()->decodeCS16Instr(instrValue);
                                rd = fields[5] | 0x8;
                                rs1 = fields[3] | 0x8;
                                uimm = (fields[4] << 6) | (fields[2] << 3);
                                // ld rd ′ , offset[7:3](rs1 ′ )
                                new_instr =
                                    (uimm << 20) | (rs1 << 15) | (0b011 << 12) | (rd << 7) | RVISA::Opcode::LOAD;
                            }
                            // else{// c.flw RV32FC-only }
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
                            if (m_isa->isaID() == ISA::RV64I) {  // c.sd
                                const auto fields = RVInstrParser::getParser()->decodeCS16Instr(instrValue);
                                rs1 = fields[3] | 0x8;
                                rs2 = fields[5] | 0x8;
                                uimm = (fields[4] << 6) | (fields[2] << 3);
                                // sd rs2 ′ ,offset[7:3](rs1 ′ )
                                new_instr = (((uimm & 0xFE0) >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (0b011 << 12) |
                                            ((uimm & 0x1F) << 7) | RVISA::Opcode::STORE;
                            }
                            // else { c.fsw RV32FC-only}
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
                            if (fields[2]) {  // test for negative
                                imm = imm | 0xFFFFFFE0;
                            }
                            // addi rd, rd, nzimm[5:0]
                            new_instr = (imm << 20) | (rd << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                        } break;
                        case 0b001:
                            if (m_isa->isaID() == ISA::RV32I) {  // c.jal
                                const auto fields = RVInstrParser::getParser()->decodeCJ16Instr(instrValue);
                                imm = (((fields[2] & 0x040) << 3) | (fields[2] & 0x180) | ((fields[2] & 0x010) << 2) |
                                       (fields[2] & 0x020) | ((fields[2] & 0x001) << 4) | ((fields[2] & 0x200) >> 6) |
                                       ((fields[2] & 0x00E) >> 1));
                                if (fields[2] & 0x400) {
                                    imm = imm | 0xFFE00;
                                }
                                // jal x1,offset[11:1]
                                new_instr = ((((imm & 0x003FF) << 9) | ((imm & 0x00400) >> 2) |
                                              ((imm & 0x7F800) >> 11) | (imm & 0x80000))
                                             << 12) |
                                            (0b00001 << 7) | RVISA::Opcode::JAL;
                            } else {  // c.addiw;
                                const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                                rd = fields[3];
                                imm = fields[4];
                                if (fields[2]) {  // test for negative
                                    imm = imm | 0xFFFFFFE0;
                                }
                                // addiw rd, rd, imm[5:0]
                                new_instr =
                                    (imm << 20) | (rd << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OPIMM32;
                            }
                            break;
                        case 0b010:  // C.LI
                        {
                            const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                            // addi rd,x0, imm[5:0]
                            rd = fields[3];
                            imm = fields[4];
                            if (fields[2]) {  // test for negative
                                imm = imm | 0xFFFFFFE0;
                            }
                            new_instr = (imm << 20) | (rd << 7) | RVISA::Opcode::OPIMM;
                            break;
                        }
                        case 0b011: {
                            const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                            rd = fields[3];
                            if (rd == 2) {  // c.addi16sp
                                imm = (((fields[4] & 0x06) << 2) | ((fields[4] & 0x08) >> 1) |
                                       ((fields[4] & 0x01) << 1) | ((fields[4] & 0x10) >> 4))
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
                                    new_instr =
                                        (uimm << 20) | (rd << 15) | (0b101 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                                } break;
                                case 0b01: {  // c.srai
                                    const auto fieldscb = RVInstrParser::getParser()->decodeCB216Instr(instrValue);
                                    uimm = (fieldscb[2] << 6) | fieldscb[5];
                                    // srai rd ′ , rd ′ , shamt[5:0]
                                    new_instr = (0b0100000 << 25) | (uimm << 20) | (rd << 15) | (0b101 << 12) |
                                                (rd << 7) | RVISA::Opcode::OPIMM;
                                } break;
                                case 0b10: {  // c.andi
                                    const auto fieldscb = RVInstrParser::getParser()->decodeCB216Instr(instrValue);
                                    imm = fieldscb[5];
                                    if (fieldscb[2]) {
                                        imm = 0xFE0 | imm;
                                    }
                                    // andi rd ′ ,rd ′ , imm[5:0]
                                    new_instr =
                                        (imm << 20) | (rd << 15) | (0b111 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
                                } break;
                                case 0b11:
                                    switch (fields[2] << 2 | fields[5]) {
                                        case 0b000:  // c.sub
                                            new_instr = (0b0100000 << 25) | (rs2 << 20) | (rd << 15) | (0b000 << 12) |
                                                        (rd << 7) | RVISA::Opcode::OP;
                                            break;
                                        case 0b001:  // c.xor
                                            new_instr = (rs2 << 20) | (rd << 15) | (0b100 << 12) | (rd << 7) |
                                                        RVISA::Opcode::OP;
                                            break;
                                        case 0b010:  // c.or
                                            new_instr = (rs2 << 20) | (rd << 15) | (0b110 << 12) | (rd << 7) |
                                                        RVISA::Opcode::OP;
                                            break;
                                        case 0b011:  // c.and
                                            new_instr = (rs2 << 20) | (rd << 15) | (0b111 << 12) | (rd << 7) |
                                                        RVISA::Opcode::OP;
                                            break;
                                        case 0b100:  // c.subw RV64C/RV128C-only
                                            new_instr = (0b0100000 << 25) | (rs2 << 20) | (rd << 15) | (0b000 << 12) |
                                                        (rd << 7) | RVISA::Opcode::OP32;
                                            break;
                                        case 0b101:  // c.addw RV64C/RV128C-only
                                            new_instr = (rs2 << 20) | (rd << 15) | (0b000 << 12) | (rd << 7) |
                                                        RVISA::Opcode::OP32;
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
                            imm = (((fields[2] & 0x040) << 3) | (fields[2] & 0x180) | ((fields[2] & 0x010) << 2) |
                                   (fields[2] & 0x020) | ((fields[2] & 0x001) << 4) | ((fields[2] & 0x200) >> 6) |
                                   ((fields[2] & 0x00E) >> 1));
                            if (fields[2] & 0x400) {
                                imm = imm | 0xFFE00;
                            }
                            // jal x0,offset[11:1]
                            new_instr = ((((imm & 0x003FF) << 9) | ((imm & 0x00400) >> 2) | ((imm & 0x7F800) >> 11) |
                                          (imm & 0x80000))
                                         << 12) |
                                        (0b00000 << 7) | RVISA::Opcode::JAL;
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
                            new_instr = ((((imm & 0x8000) >> 9) | ((imm & 0x3F00) >> 8)) << 25) | (0b00 << 20) |
                                        (rs1 << 15) | (0b000 << 12) |
                                        ((((imm & 0x000F) << 1) | ((imm & 0x4000) >> 14)) << 7) | RVISA::Opcode::BRANCH;
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
                            new_instr = ((((imm & 0x8000) >> 9) | ((imm & 0x3F00) >> 8)) << 25) | (0b00 << 20) |
                                        (rs1 << 15) | (0b001 << 12) |
                                        ((((imm & 0x000F) << 1) | ((imm & 0x4000) >> 14)) << 7) | RVISA::Opcode::BRANCH;
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
                                new_instr =
                                    (uimm << 20) | (rd << 15) | (0b001 << 12) | (rd << 7) | RVISA::Opcode::OPIMM;
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
                            if (m_isa->isaID() == ISA::RV64I) {  // c.ldsp
                                const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                                rd = fields[3];
                                uimm = ((fields[4] & 0x07) << 6) | (fields[2] << 5) | (fields[4] & 0x18);
                                // ld rd,offset[8:3](x2)
                                new_instr =
                                    (uimm << 20) | (0b0010 << 15) | (0b011 << 12) | (rd << 7) | RVISA::Opcode::LOAD;
                            }
                            // else{// c.flwsp RV32FC-only}
                            break;
                        case 0b100: {
                            const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                            rd = fields[3];
                            rs2 = fields[4];
                            if (fields[2]) {
                                if (rs2) {  // c.add
                                    // add rd, rd, rs2
                                    new_instr =
                                        (rs2 << 20) | (rd << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OP;
                                } else {
                                    if (rd) {  // c.jarl
                                        // jalr x1, 0(rs1)
                                        new_instr = (0b0 << 20) | (rd << 15) | (0b000 << 12) | (0b00001 << 7) |
                                                    RVISA::Opcode::JALR;
                                    }
                                    // else{
                                    // c.ebreak  -> ebreak  Not implemented in Ripes
                                    //}
                                }
                            } else {
                                if (rs2) {  // c.mv
                                            // add rd, x0, rs2
                                    new_instr =
                                        (rs2 << 20) | (0b0 << 15) | (0b000 << 12) | (rd << 7) | RVISA::Opcode::OP;
                                } else {  // c.jr
                                    // jalr x0, 0(rs1)
                                    new_instr =
                                        (0b0 << 20) | (rd << 15) | (0b000 << 12) | (0b00000 << 7) | RVISA::Opcode::JALR;
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
                            if (m_isa->isaID() == ISA::RV64I) {  // c.sdsp
                                const auto fields = RVInstrParser::getParser()->decodeCSS16Instr(instrValue);
                                rs2 = fields[3];
                                uimm = ((fields[2] & 0x07) << 6) | (fields[2] & 0x38);
                                // sd rs2,offset[8:3](x2)
                                new_instr = (((uimm & 0xFE0) >> 5) << 25) | (rs2 << 20) | (0b00010 << 15) |
                                            (0b011 << 12) | ((uimm & 0x1F) << 7) | RVISA::Opcode::STORE;
                            }
                            // else{// c.fswsp RV32FC-only}
                            break;
                    }
                    break;
                default:  // No compressed
                    break;
            }

            return new_instr;
        };
    }

    INPUTPORT(instr, c_RVInstrWidth);
    OUTPUTPORT(Pc_Inc, 1);
    OUTPUTPORT(exp_instr, c_RVInstrWidth);

private:
    std::shared_ptr<ISAInfoBase> m_isa;
    bool m_disabled = true;
};

}  // namespace core
}  // namespace vsrtl
