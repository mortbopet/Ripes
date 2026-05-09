#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "riscv.h"

#include "isa/rv_f_ext.h"
#include "isa/rv_zicsr_ext.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class Decode : public Component {
public:
  void setISA(const std::shared_ptr<ISAInfoBase> &isa) { m_isa = isa; }

  Decode(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    opcode << [this] {
      const auto instrValue = instr.uValue();

      const unsigned l7 = instrValue & 0b1111111;

      // clang-format off
      switch(l7) {
        case RVISA::OpcodeID::LUI:    return RVInstr::LUI;
        case RVISA::OpcodeID::AUIPC:  return RVInstr::AUIPC;
        case RVISA::OpcodeID::JAL:    return RVInstr::JAL;
        case RVISA::OpcodeID::JALR:   return RVInstr::JALR;
        case RVISA::OpcodeID::SYSTEM:  {
          const auto fields = RVInstrParser::getParser()->decodeI32Instr(instrValue);
          RVISA::ExtZicsr::TypeCSR::Funct3 funct3 = static_cast<RVISA::ExtZicsr::TypeCSR::Funct3>(fields[2]);

          if (funct3 == RVISA::ExtZicsr::TypeCSR::Funct3::ECALL) {
            return RVInstr::ECALL;
          }

          if(!m_isa || !m_isa->extensionEnabled(RVISA::Extension::Zicsr)) {
            // Fallthrough - unknown instruction.
            break;
          }
          
          // general decode csr instructions
          switch (funct3) {
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRW:  return RVInstr::CSRRW;
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRS:  return RVInstr::CSRRS;
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRC:  return RVInstr::CSRRC;
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRWI: return RVInstr::CSRRWI;
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRSI: return RVInstr::CSRRSI;
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRCI: return RVInstr::CSRRCI;
            default: break;
          }
          break;
        }
        

        case RVISA::OpcodeID::OPIMM: {
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

        case RVISA::OpcodeID::OPIMM32: {
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

        case RVISA::OpcodeID::OP: {
          // R-Type
          const auto fields = RVInstrParser::getParser()->decodeR32Instr(instrValue);
          if (fields[0] == 0b0000001) {
            if(m_isa && m_isa->extensionEnabled(RVISA::Extension::M)) {
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

        case RVISA::OpcodeID::OP32: {
          // R-Type (32-bit, in 64-bit ISA)
          const auto fields = RVInstrParser::getParser()->decodeR32Instr(instrValue);
          if (fields[0] == 0b0000001) {
            if(m_isa && m_isa->extensionEnabled(RVISA::Extension::M)) {
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

        case RVISA::OpcodeID::LOAD: {
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

        case RVISA::OpcodeID::STORE: {
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

        case RVISA::OpcodeID::BRANCH: {
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

      if(!m_isa || !m_isa->extensionEnabled(RVISA::Extension::F)) {
        // Fallthrough - unknown instruction.
        return RVInstr::NOP;
      }

      // floating point opcode IDs
      switch(l7) {
        case RVISA::OpcodeID::LOAD_FP: {
          const auto fields = RVInstrParser::getParser()->decodeI32Instr(instrValue);
          switch(static_cast<RVISA::ExtF::Width>(fields[2])) { // funct3, here Width W
            case RVISA::ExtF::Width::W: return RVInstr::FLW;
            default: break;
          }
          break;
        }

        case RVISA::OpcodeID::STORE_FP: {
            const auto fields = RVInstrParser::getParser()->decodeS32Instr(instrValue);
            switch(static_cast<RVISA::ExtF::Width>(fields[3])) { // funct3, here Width W
                case RVISA::ExtF::Width::W: return RVInstr::FSW;
                default: break;
            }
            break;
        }

        case RVISA::OpcodeID::OP_FP: {
          namespace TypeR = RVISA::ExtF::TypeR;

          const auto fields = RVInstrParser::getParser()->decodeR32Instr(instrValue);
          RVISA::ExtF::RoundMode rm = static_cast<RVISA::ExtF::RoundMode>(fields[3]);
          RVISA::ExtF::FMT fmt = static_cast<RVISA::ExtF::FMT>(fields[0] & 0b11);

          if(fmt != RVISA::ExtF::FMT::S) {
            break; // only single float instructions implemented so far
          }
          
          auto rs2 = fields[1];
          TypeR::Funct5 funct5 = static_cast<TypeR::Funct5>((fields[0] >> 2) & 0b11111);
          switch(funct5) {
            case TypeR::Funct5::FADD:  return RVInstr::FADD_S;
            case TypeR::Funct5::FSUB:  return RVInstr::FSUB_S;
            case TypeR::Funct5::FMUL:  return RVInstr::FMUL_S;
            case TypeR::Funct5::FDIV:  return RVInstr::FDIV_S;
            case TypeR::Funct5::FSQRT: return RVInstr::FSQRT_S;

            case TypeR::Funct5::FMIN_MAX: {
              switch(static_cast<RVISA::ExtF::funct3_t>(rm)) {
                case TypeR::f3_MIN: return RVInstr::FMIN_S;
                case TypeR::f3_MAX: return RVInstr::FMAX_S;
                default: break; // invalid
              }
              break;
            }

            case TypeR::Funct5::FSGNJ: {
              switch(static_cast<TypeR::SgnJ>(rm)) {
                case TypeR::SgnJ::FSGNJ:  return RVInstr::FSGNJ_S;
                case TypeR::SgnJ::FSGNJN: return RVInstr::FSGNJN_S;
                case TypeR::SgnJ::FSGNJX: return RVInstr::FSGNJX_S;
                default: break; // invalid
              }
              break;
            }

            case TypeR::Funct5::FCVT_FMT_INT: {
              switch(static_cast<TypeR::Cvt_Int>(rs2)) {
                case TypeR::Cvt_Int::W:  return RVInstr::FCVT_S_W;
                case TypeR::Cvt_Int::WU: return RVInstr::FCVT_S_WU;
                case TypeR::Cvt_Int::L:  return RVInstr::FCVT_S_L;
                case TypeR::Cvt_Int::LU: return RVInstr::FCVT_S_LU;
                default: break;
              }
              break;
            }
            case TypeR::Funct5::FCVT_INT_FMT: {
              switch(static_cast<TypeR::Cvt_Int>(rs2)) {
                case TypeR::Cvt_Int::W:  return RVInstr::FCVT_W_S;
                case TypeR::Cvt_Int::WU: return RVInstr::FCVT_WU_S;
                case TypeR::Cvt_Int::L:  return RVInstr::FCVT_L_S;
                case TypeR::Cvt_Int::LU: return RVInstr::FCVT_LU_S;
                default: break;
              }
              break;
            }
            
            // same as fclass.s
            case TypeR::Funct5::FMV_X_W: {
              switch(static_cast<unsigned>(rm)) {
                case 0b000: return RVInstr::FMV_X_W;
                case 0b001: return RVInstr::FCLASS_S;
                default: break; // invalid
              }
              break;
            }
            case TypeR::Funct5::FMV_W_X: return RVInstr::FMV_W_X;

            case TypeR::Funct5::FCMP: {
              switch(static_cast<TypeR::Comp>(rm)) {
                case TypeR::Comp::EQ: return RVInstr::FEQ_S;
                case TypeR::Comp::LT: return RVInstr::FLT_S;
                case TypeR::Comp::LE: return RVInstr::FLE_S;
                default: break; // invalid
              }
              break;
            }
          }
          break;
        }
          
        case RVISA::OpcodeID::MADD: {
          const auto fields = RVInstrParser::getParser()->decodeR432Instr(instrValue);
          
          switch(static_cast<RVISA::ExtF::FMT>(fields[1])) { // fmt
            case RVISA::ExtF::FMT::S: return RVInstr::FMADD_S;
            default: break;
          }
          break;
        }

        case RVISA::OpcodeID::MSUB: {
          const auto fields = RVInstrParser::getParser()->decodeR432Instr(instrValue);
          
          switch(static_cast<RVISA::ExtF::FMT>(fields[1])) { // fmt
            case RVISA::ExtF::FMT::S: return RVInstr::FMSUB_S;
            default: break;
          }
          break;
        }

        case RVISA::OpcodeID::NMSUB: {
          const auto fields = RVInstrParser::getParser()->decodeR432Instr(instrValue);
          
          switch(static_cast<RVISA::ExtF::FMT>(fields[1])) { // fmt
            case RVISA::ExtF::FMT::S: return RVInstr::FNMSUB_S;
            default: break;
          }
          break;
        }

        case RVISA::OpcodeID::NMADD: {
          const auto fields = RVInstrParser::getParser()->decodeR432Instr(instrValue);
          
          switch(static_cast<RVISA::ExtF::FMT>(fields[1])) { // fmt
            case RVISA::ExtF::FMT::S: return RVInstr::FNMADD_S;
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

    wr_reg_idx << [this] {
      return (instr.uValue() >> 7) & 0b11111;
    };

    r1_reg_idx << [this] {
        const auto instrValue = instr.uValue();
        
        // CSRRWI/CSRRSI/CSRRCI instructions use Rs1 as an immediate value instead of a register index
        // this must be handled separately as they are still grouped as I-Type instructions under the SYSTEM opcodes
        const unsigned l7 = instrValue & 0b1111111;
        if( l7 == RVISA::OpcodeID::SYSTEM && m_isa && m_isa->extensionEnabled(RVISA::Extension::Zicsr) ) {
          const auto fields = RVInstrParser::getParser()->decodeI32Instr(instrValue);
          RVISA::ExtZicsr::TypeCSR::Funct3 funct3 = static_cast<RVISA::ExtZicsr::TypeCSR::Funct3>(fields[2]);
          
          // general decode csr instructions
          switch (funct3) {
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRWI: 
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRSI: 
            case RVISA::ExtZicsr::TypeCSR::Funct3::CSRRCI: 
                return VSRTL_VT_U(0); // immediate value, not register index
            
            default: break; // for other SYSTEM instructions, Rs1 is still a register index and should be decoded as normal below
          }
        }

        const RVInstrType instr_type = getInstrType(instrValue);
        
        // Bits 15-19 NOT defined as Rs1 for U and J formats.
        if (instr_type == RVInstrType::U || instr_type == RVInstrType::J) {
            return vsrtl::VSRTL_VT_U(0);
        }
        else {
            return (instrValue >> 15) & 0b11111;
        }
    };

    r2_reg_idx << [this] {
        const auto instrValue = instr.uValue();
        const RVInstrType instr_type = getInstrType(instrValue);

        // Bits 20-24 defined as Rs2 only for S, R, R4 and B formats.
        if (   instr_type == RVInstrType::S 
            || instr_type == RVInstrType::R 
            || instr_type == RVInstrType::R4 
            || instr_type == RVInstrType::B 
        ) {
            return (instrValue >> 20) & 0b11111;
        }
        else {
            return vsrtl::VSRTL_VT_U(0);
        }
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

template <unsigned XLEN>
class DecodeCSR : public Decode<XLEN> {
  public:
  DecodeCSR(const std::string &name, SimComponent *parent)
  : Decode<XLEN>(name, parent) {
    csr_reg_idx << [this] { // csr operations are atomic so no hazards with other instructions
      return (Decode<XLEN>::instr.uValue() >> 20) & 0xFFF; // 12 bits
    };
  }
  // OUTPUTPORT_ENUM(csr_reg_idx, RVISA::RV_CSRInfo::CSR);
  OUTPUTPORT(csr_reg_idx, c_RVCsrRegsBits);
};

template <unsigned XLEN>
class DecodeF : public DecodeCSR<XLEN> {
  public:
  DecodeF(const std::string &name, SimComponent *parent)
  : DecodeCSR<XLEN>(name, parent) {
    r3_reg_idx << [this] {
      const auto instrValue = Decode<XLEN>::instr.uValue();
      const RVInstrType instr_type = getInstrType(instrValue);

      // Bits 27-31 defined as Rs3 only for R4 format.
      if ( instr_type == RVInstrType::R4 ) {
        return (instrValue >> 27) & 0b11111;
      } else {
        return vsrtl::VSRTL_VT_U(0);
      }
    };
    roundMode << [this] {
      return (Decode<XLEN>::instr.uValue() >> 12) & 0b111;
    };
  }
  OUTPUTPORT(r3_reg_idx, c_RVRegsBits);
  OUTPUTPORT_ENUM(roundMode, RVISA::ExtF::RoundMode);
};

} // namespace core
} // namespace vsrtl
