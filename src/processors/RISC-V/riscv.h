#pragma once

#include <functional>

#include "rv_instrparser.h"

#include "isa/rvisainfo_common.h"

namespace Ripes {

// namespace RVISA {

// template <unsigned XLEN>
// ProcessorISAInfo supportsISA() {
//   using RVISAInfo = ISAInfo<XLenToRVISA<XLEN>()>;
//   return ProcessorISAInfo{
//     std::make_shared<RVISAInfo>(QStringList()),
//     std::make_shared<RV_ExtensionSet>(RVISAInfo::getSupportedExtensions()),
//     std::make_shared<RV_ExtensionSet>(RVISAInfo::getDefaultExtensions())};
// }

// template <unsigned XLEN>
// std::shared_ptr<const ISAInfoBase> fullISA() {
//   return ISAInfoRegistry::getSupportedISA<XLenToRVISA<XLEN>()>();
// }

// } // namespace RVISA


/** Instruction set enumerations */
enum class RVInstrType { R, I, S, B, U, J, R4 };

enum class RVInstr {
  NOP,
  /* RV32I Base Instruction Set */
  LUI,
  AUIPC,
  JAL,
  JALR,
  BEQ,
  BNE,
  BLT,
  BGE,
  BLTU,
  BGEU,
  LB,
  LH,
  LW,
  LBU,
  LHU,
  SB,
  SH,
  SW,
  ADDI,
  SLTI,
  SLTIU,
  XORI,
  ORI,
  ANDI,
  SLLI,
  SRLI,
  SRAI,
  ADD,
  SUB,
  SLL,
  SLT,
  SLTU,
  XOR,
  SRL,
  SRA,
  OR,
  AND,
  ECALL,

  /* RV32M Standard Extension */
  MUL,
  MULH,
  MULHSU,
  MULHU,
  DIV,
  DIVU,
  REM,
  REMU,

  /* RV64I Base Instruction Set */
  ADDIW,
  SLLIW,
  SRLIW,
  SRAIW,
  ADDW,
  SUBW,
  SLLW,
  SRLW,
  SRAW,
  LWU,
  LD,
  SD,

  /* RV64M Standard Extension */
  MULW,
  DIVW,
  DIVUW,
  REMW,
  REMUW,

  /* RV32F Single Floating Point Extension */
  FLW,
  FSW,
  
  FADD_S,
  FSUB_S,
  FMUL_S,
  FDIV_S,
  FSQRT_S,
  FMIN_S,
  FMAX_S,
  
  FSGNJ_S,
  FSGNJN_S,
  FSGNJX_S,
  
  FCVT_W_S,
  FCVT_WU_S,
  FCVT_S_W,
  FCVT_S_WU,

  FMV_X_W,
  FMV_W_X,
  
  FEQ_S,
  FLT_S,
  FLE_S,

  FCLASS_S,

  FMADD_S,
  FMSUB_S,
  FNMSUB_S,
  FNMADD_S,
  
  /* RV64F Single Floating Point Extension */
  FCVT_L_S,
  FCVT_LU_S,
  FCVT_S_L,
  FCVT_S_LU,

  /* RV32_Zicsr - Control and status registers Extension */
  CSRRW,
  CSRRS,
  CSRRC,
  CSRRWI,
  CSRRSI,
  CSRRCI
};

[[maybe_unused]]
static inline RVISA::Extension::Id getExtensionType(const RVInstr instr) {
  switch (instr) {
    case RVInstr::MUL:    case RVInstr::MULH:
    case RVInstr::MULHSU: case RVInstr::MULHU:
    case RVInstr::DIV:    case RVInstr::DIVU:
    case RVInstr::REM:    case RVInstr::REMU:
    case RVInstr::MULW:   
    case RVInstr::DIVW:   case RVInstr::DIVUW:  
    case RVInstr::REMW:   case RVInstr::REMUW:
      return RVISA::Extension::Id::M;

    case RVInstr::FLW:      case RVInstr::FSW:
    case RVInstr::FADD_S:   case RVInstr::FSUB_S:
    case RVInstr::FMUL_S:   case RVInstr::FDIV_S:
    case RVInstr::FSQRT_S: 
    case RVInstr::FMIN_S:   case RVInstr::FMAX_S: 
    case RVInstr::FSGNJ_S:  case RVInstr::FSGNJN_S: case RVInstr::FSGNJX_S:
    case RVInstr::FCVT_W_S: case RVInstr::FCVT_WU_S:
    case RVInstr::FCVT_L_S: case RVInstr::FCVT_LU_S:
    case RVInstr::FCVT_S_W: case RVInstr::FCVT_S_WU:
    case RVInstr::FCVT_S_L: case RVInstr::FCVT_S_LU:
    case RVInstr::FMV_X_W:  case RVInstr::FMV_W_X:
    case RVInstr::FEQ_S:    case RVInstr::FLT_S: case RVInstr::FLE_S:
    case RVInstr::FMADD_S:  case RVInstr::FMSUB_S: 
    case RVInstr::FNMSUB_S: case RVInstr::FNMADD_S:
    case RVInstr::FCLASS_S:
      return RVISA::Extension::Id::F;
    
    case RVInstr::CSRRW:  case RVInstr::CSRRS:  case RVInstr::CSRRC:
    case RVInstr::CSRRWI: case RVInstr::CSRRSI: case RVInstr::CSRRCI:
      return RVISA::Extension::Id::Zicsr;

    default:
      return RVISA::Extension::Id::I;
  }
}

[[maybe_unused]]
static RVInstrType getInstrType(unsigned instrValue) {
  const unsigned l7 = instrValue & 0b1111111;

  switch(l7) {
    case RVISA::OpcodeID::LUI:      return RVInstrType::U;
    case RVISA::OpcodeID::AUIPC:    return RVInstrType::U;
    case RVISA::OpcodeID::JAL:      return RVInstrType::J;
    case RVISA::OpcodeID::JALR:     return RVInstrType::I;
    case RVISA::OpcodeID::SYSTEM:   return RVInstrType::I;
    case RVISA::OpcodeID::OPIMM:    return RVInstrType::I;
    case RVISA::OpcodeID::OPIMM32:  return RVInstrType::I;
    case RVISA::OpcodeID::LOAD:     return RVInstrType::I;
    case RVISA::OpcodeID::OP:       return RVInstrType::R;
    case RVISA::OpcodeID::OP32:     return RVInstrType::R;
    case RVISA::OpcodeID::STORE:    return RVInstrType::S;
    case RVISA::OpcodeID::BRANCH:   return RVInstrType::B;
    
    // Fused Multiply and Add
    case RVISA::OpcodeID::MADD :    return RVInstrType::R4;
    case RVISA::OpcodeID::MSUB :    return RVInstrType::R4;
    case RVISA::OpcodeID::NMSUB:    return RVInstrType::R4;
    case RVISA::OpcodeID::NMADD:    return RVInstrType::R4;
  
    // Floating point extension
    case RVISA::OpcodeID::LOAD_FP:  return RVInstrType::I;
    case RVISA::OpcodeID::STORE_FP: return RVInstrType::S;
    case RVISA::OpcodeID::OP_FP:    return RVInstrType::R;

    default:                        return RVInstrType::I; // Default - unknown instruction (NOP).
  }
}

/** Datapath enumerations */
enum class ALUOp {
  NOP,
  ADD,
  SUB,
  MUL,
  DIV,
  AND,
  OR,
  XOR,
  SL,
  SRA,
  SRL,
  LUI,
  LT,
  LTU,
  EQ,
  MULH,
  MULHU,
  MULHSU,
  DIVU,
  REM,
  REMU,
  SLW,
  SRLW,
  SRAW,
  ADDW,
  SUBW,
  MULW,
  DIVW,
  DIVUW,
  REMW,
  REMUW
};
enum class FPUOp {
  NOP,
  
  /* RV32F Single Floating Point Extension */
  FADD_S,
  FSUB_S,
  FMUL_S,
  FDIV_S,
  FSQRT_S,
  FMIN_S,
  FMAX_S,
  
  FMADD_S,
  FMSUB_S,
  FNMSUB_S,
  FNMADD_S,
  
  FSGNJ_S,
  FSGNJN_S,
  FSGNJX_S,
  
  FCVT_W_S,
  FCVT_WU_S,
  FCVT_S_W,
  FCVT_S_WU,

  FMV_X_W,
  FMV_W_X,
  
  EQ,
  LE,
  LT,

  FCLASS_S,

  /* RV64F Single Floating Point Extension */
  FCVT_L_S,
  FCVT_LU_S,
  FCVT_S_L,
  FCVT_S_LU,

  /* Floating Point - Control and status registers instructions */
  CSRW_FFLAGS,
  CSRS_FFLAGS,
  CSRC_FFLAGS,
  
  CSRW_FRM,
  CSRS_FRM,
  CSRC_FRM,

  CSRW_FCSR,
  CSRS_FCSR,
  CSRC_FCSR,
};
enum class RegWrSrc { MEMREAD, ALURES, PC4 };
enum class AluSrc1 { REG1, PC };
enum class AluSrc2 { REG2, IMM };
enum class CompOp { NOP, EQ, NE, LT, LTU, GE, GEU };
enum class MemOp { NOP, LB, LH, LW, LBU, LHU, SB, SH, SW, LWU, LD, SD };
enum ECALL { none, print_int = 1, print_char = 2, print_string = 4, exit = 10 };
enum PcSrc { PC4 = 0, ALU = 1 };
enum PcInc { INC2 = 0, INC4 = 1 };
enum class RegFileSrc { INTEGER, FLOAT };
enum class ImmRegFileSrc { INTEGER, FLOAT, IMMEDIATE };

/** Instruction field parser */
class RVInstrParser {
public:
  static RVInstrParser *getParser() {
    static RVInstrParser parser;
    return &parser;
  }

  std::vector<uint32_t> decodeU32Instr(const uint32_t &instr) const {
    return m_decodeU32Instr(instr);
  }
  std::vector<uint32_t> decodeJ32Instr(const uint32_t &instr) const {
    return m_decodeJ32Instr(instr);
  }
  std::vector<uint32_t> decodeI32Instr(const uint32_t &instr) const {
    return m_decodeI32Instr(instr);
  }
  std::vector<uint32_t> decodeS32Instr(const uint32_t &instr) const {
    return m_decodeS32Instr(instr);
  }
  std::vector<uint32_t> decodeR32Instr(const uint32_t &instr) const {
    return m_decodeR32Instr(instr);
  }
  std::vector<uint32_t> decodeB32Instr(const uint32_t &instr) const {
    return m_decodeB32Instr(instr);
  }

  // RVF
  std::vector<uint32_t> decodeR432Instr(const uint32_t &instr) const {
    return m_decodeR432Instr(instr);
  }

  // RVC
  std::vector<uint32_t> decodeCA16Instr(const uint32_t &instr) const {
    return m_decodeCA16Instr(instr);
  }
  std::vector<uint32_t> decodeCI16Instr(const uint32_t &instr) const {
    return m_decodeCI16Instr(instr);
  }
  std::vector<uint32_t> decodeCS16Instr(const uint32_t &instr) const {
    return m_decodeCS16Instr(instr);
  }
  std::vector<uint32_t> decodeCIW16Instr(const uint32_t &instr) const {
    return m_decodeCIW16Instr(instr);
  }
  std::vector<uint32_t> decodeCSS16Instr(const uint32_t &instr) const {
    return m_decodeCSS16Instr(instr);
  }
  std::vector<uint32_t> decodeCJ16Instr(const uint32_t &instr) const {
    return m_decodeCJ16Instr(instr);
  }
  std::vector<uint32_t> decodeCB16Instr(const uint32_t &instr) const {
    return m_decodeCB16Instr(instr);
  }
  std::vector<uint32_t> decodeCB216Instr(const uint32_t &instr) const {
    return m_decodeCB216Instr(instr);
  }

private:
  RVInstrParser() {
    // from LSB to MSB
    m_decodeR32Instr =
        generateInstrParser<uint32_t>(std::vector<int>{7, 5, 3, 5, 5, 7});
    m_decodeI32Instr =
        generateInstrParser<uint32_t>(std::vector<int>{7, 5, 3, 5, 12});
    m_decodeS32Instr =
        generateInstrParser<uint32_t>(std::vector<int>{7, 5, 3, 5, 5, 7});
    m_decodeB32Instr =
        generateInstrParser<uint32_t>(std::vector<int>{7, 1, 4, 3, 5, 5, 6, 1});
    m_decodeU32Instr =
        generateInstrParser<uint32_t>(std::vector<int>{7, 5, 20});
    m_decodeJ32Instr =
        generateInstrParser<uint32_t>(std::vector<int>{7, 5, 8, 1, 10, 1});

    // RVF
    m_decodeR432Instr =
        generateInstrParser<uint32_t>(std::vector<int>{7, 5, 3, 5, 5, 2, 5});
    
    
    // RVC
    m_decodeCA16Instr = generateInstrParser<uint32_t>(
        std::vector<int>{2, 3, 2, 3, 2, 1, 3, 16});
    m_decodeCI16Instr =
        generateInstrParser<uint32_t>(std::vector<int>{2, 5, 5, 1, 3, 16});
    m_decodeCS16Instr =
        generateInstrParser<uint32_t>(std::vector<int>{2, 3, 2, 3, 3, 3, 16});
    m_decodeCIW16Instr =
        generateInstrParser<uint32_t>(std::vector<int>{2, 3, 8, 3, 16});
    m_decodeCSS16Instr =
        generateInstrParser<uint32_t>(std::vector<int>{2, 5, 6, 3, 16});
    m_decodeCJ16Instr =
        generateInstrParser<uint32_t>(std::vector<int>{2, 11, 3, 16});
    m_decodeCB16Instr =
        generateInstrParser<uint32_t>(std::vector<int>{2, 5, 3, 3, 3, 16});
    m_decodeCB216Instr =
        generateInstrParser<uint32_t>(std::vector<int>{2, 5, 3, 2, 1, 3, 16});
  }
  decode_functor<uint32_t> m_decodeU32Instr;
  decode_functor<uint32_t> m_decodeJ32Instr;
  decode_functor<uint32_t> m_decodeI32Instr;
  decode_functor<uint32_t> m_decodeS32Instr;
  decode_functor<uint32_t> m_decodeR32Instr;
  decode_functor<uint32_t> m_decodeB32Instr;

  // RVF
  decode_functor<uint32_t> m_decodeR432Instr;

  // RVC
  decode_functor<uint32_t> m_decodeCA16Instr;
  decode_functor<uint32_t> m_decodeCI16Instr;
  decode_functor<uint32_t> m_decodeCS16Instr;
  decode_functor<uint32_t> m_decodeCIW16Instr;
  decode_functor<uint32_t> m_decodeCSS16Instr;
  decode_functor<uint32_t> m_decodeCJ16Instr;
  decode_functor<uint32_t> m_decodeCB16Instr;
  decode_functor<uint32_t> m_decodeCB216Instr;
};

} // namespace Ripes
