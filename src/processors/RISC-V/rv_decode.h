#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;


//only support 32 bit instructions
unsigned long int 
uncompress(const unsigned long int instrValue)
{
    static unsigned long int instr_last  = 0;
    static unsigned long int instr_cache = 0;
    
    //use cache to avoid run multiple times with same instruction in multiple calls
    if(instr_last == instrValue)
    {
        return instr_cache;
    }
    
    unsigned long int new_instr= instrValue;
    int imm; 
    int rd;
    int rs2;
    
    const int quadrant = instrValue & 0b11;
    const int func3    = (instrValue & 0xE000)>> 13;

         
            switch(quadrant)
            {
                case 0x00:
                    switch(func3)
                    {
                        case 0b000:
                            break;
                        case 0b001:
                            break;
                        case 0b010:
                            break;
                        case 0b011:
                            break;
                        case 0b100://RESERVED
                            break;
                        case 0b101:
                            break;
                        case 0b110:
                            break;
                        case 0b111:
                            break;
                    }
                    break;
                case 0x01:
                    switch(func3)
                    {
                        case 0b000:
                            break;
                        case 0b001:
                            break;
                        case 0b010://C.LI
                        {
                            const auto fields = RVInstrParser::getParser()->decodeCI16Instr(instrValue);
                            //addi rd,x0, imm[5:0]
                            rd = fields[3];
                            imm = (fields[2] << 5)|fields[4];
                            if( imm & 0x20) //test for negative
                            {
                                imm = imm | 0xFFFFFFC0;
                            }
                            new_instr =  (imm << 20)|(rd<<7) |RVInstr::ADDI;
                            break;
                        }
                        case 0b011:
                            break;
                        case 0b100://MISC-ALU
                        {
                            const auto fields = RVInstrParser::getParser()->decodeCA16Instr(instrValue);
                            rd = fields[4]  |0x8;
                            rs2 = fields[6] |0x8;
                            switch(fields[3])
                            {
                                case 0b00:
                                    break;
                                case 0b01:
                                    break;
                                case 0b10:
                                    break;
                                case 0b11:
                                    switch(fields[2]<<2|fields[5])
                                    {
                                        case 0b000://c.sub 
                                            new_instr = (0b0100000 << 25)|(rs2 << 20)|(rd << 15)|(0b000 <<12) |(rd<<7) |RVISA::Opcode::OP;
                                            break;
                                        case 0b001://c.xor
                                            new_instr = (rs2 << 20)|(rd << 15)|(0b100 <<12) |(rd<<7) |RVISA::Opcode::OP;
                                            break;
                                        case 0b010://c.or
                                            new_instr = (rs2 << 20)|(rd << 15)|(0b110 <<12) |(rd<<7) |RVISA::Opcode::OP;
                                            break;
                                        case 0b011://c.and
                                            new_instr = (rs2 << 20)|(rd << 15)|(0b111 <<12) |(rd<<7) |RVISA::Opcode::OP;
                                            break;
                                        case 0b100://c.subw RV64C/RV128C-only
                                            new_instr = (0b0100000 << 25)|(rs2 << 20)|(rd << 15)|(0b000 <<12) |(rd<<7) |RVISA::Opcode::OP32;
                                            break;
                                        case 0b101://c.addw RV64C/RV128C-only
                                            new_instr = (rs2 << 20)|(rd << 15)|(0b000 <<12) |(rd<<7) |RVISA::Opcode::OP32;
                                            break;
                                        case 0b110://RESERVED
                                            break;
                                        case 0b111://RESERVED
                                            break;                                            
                                    }
                                    break;                                    
                            }
                            break;
                        }
                        case 0b101:
                            break;
                        case 0b110:
                            break;
                        case 0b111:
                            break;
                    }
                    break;        
                case 0x02:
                    switch(func3)
                    {
                        case 0b000:
                            break;
                        case 0b001:
                            break;
                        case 0b010:
                            break;
                        case 0b011:
                            break;
                        case 0b100:
                            break;
                        case 0b101:
                            break;
                        case 0b110:
                            break;
                        case 0b111:
                            break;
                    }
                    break;
                default://No compressed
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
            auto instrValue = instr.uValue();
            
            if ((instr.uValue() & 0b11) != 0b11)
            {
               instrValue = uncompress(instrValue);
            }
            
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
            if ((instr.uValue() & 0b11) != 0b11)
            {
                return (uncompress(instr.uValue())>> 7) & 0b11111;
            }
            return (instr.uValue() >> 7) & 0b11111;
        };

        r1_reg_idx << [=] {
            if ((instr.uValue() & 0b11) != 0b11)
            {
                return (uncompress(instr.uValue())>> 15) & 0b11111;
            }
            return (instr.uValue() >> 15) & 0b11111;
        };

        r2_reg_idx << [=] {
            if ((instr.uValue() & 0b11) != 0b11)
            {
                return (uncompress(instr.uValue())>> 20) & 0b11111;
            }
            return (instr.uValue() >> 20) & 0b11111;
        };

        Pc_Inc << [=] {
            if ((instr.uValue() & 0b11) != 0b11)
            {
                return 0;
            }
            return 1;
        };

        exp_instr << [=] {
            if ((instr.uValue() & 0b11) != 0b11)
            {
                return uncompress(instr.uValue());
            }
            return instr.uValue();
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
