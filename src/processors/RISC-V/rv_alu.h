#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "limits.h"
#include <assert.h>
#include <math.h>

#include "riscv.h"

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"
#include "assembler/sFloat.h"

namespace vsrtl {
namespace core {
using namespace Ripes;
using namespace Ripes::SoftFloat;
using RVISA::ExtF::RoundMode;

constexpr int32_t div_overflow32 = (-2147483648); //-2^(32-1)
constexpr int64_t div_overflow64 = (LLONG_MIN);   //-2^(64-1)

template <unsigned XLEN>
class ALU : public Component {
public:
  SetGraphicsType(ALU);
  ALU(const std::string &name, SimComponent *parent) : Component(name, parent) {
    res << [this] {
      switch (ctrl.eValue<ALUOp>()) {
      case ALUOp::ADD:
        return op1.uValue() + op2.uValue();
      case ALUOp::SUB:
        return op1.uValue() - op2.uValue();
      case ALUOp::MULW:
        return VT_U(
            signextend<32>(static_cast<int32_t>(op1.uValue()) * op2.uValue()));
      case ALUOp::MUL:
        return VT_U(op1.sValue() * op2.sValue());
      case ALUOp::MULH: {
        if constexpr (XLEN == 32) {
          const auto result = static_cast<int64_t>(op1.sValue()) *
                              static_cast<int64_t>(op2.sValue());
          return VT_U(result >> 32);
        } else {
#ifdef _MSC_VER
          // Windows (MSVC) implementation
          int64_t high;
          _mul128(op1.sValue(), op2.sValue(), &high);
          return VT_U(high);
#else
          // Linux/macOS (GCC/Clang) implementation
          const __int128_t result = static_cast<__int128_t>(op1.sValue()) *
                                    static_cast<__int128_t>(op2.sValue());
          return VT_U(result >> 64);
#endif
        }
      }
      case ALUOp::MULHU: {
        if constexpr (XLEN == 32) {
          const uint64_t result = static_cast<uint64_t>(op1.uValue()) *
                                  static_cast<uint64_t>(op2.uValue());
          return VT_U(result >> 32);
        } else {
#ifdef _MSC_VER
          // Windows (MSVC) implementation
          unsigned __int64 high;
          _umul128(op1.uValue(), op2.uValue(), &high);
          return VT_U(high);
#else
          // Linux/macOS (GCC/Clang) implementation
          const unsigned __int128 result =
              static_cast<unsigned __int128>(op1.uValue()) *
              static_cast<unsigned __int128>(op2.uValue());
          return VT_U(result >> 64);
#endif
        }
      }
      case ALUOp::MULHSU: {
        if constexpr (XLEN == 32) {
          const int64_t result = static_cast<int64_t>(op1.sValue()) *
                                 static_cast<uint64_t>(op2.uValue());
          return VT_U(result >> 32);
        } else {
#ifdef _MSC_VER
          // Windows (MSVC) implementation for Mixed Signed * Unsigned
          unsigned __int64 high;
          _umul128(static_cast<uint64_t>(op1.sValue()), op2.uValue(), &high);
          if (op1.sValue() < 0) {
            high -= op2.uValue();
          }
          return VT_U(high);
#else
          // Linux/macOS (GCC/Clang) implementation
          const __int128_t result =
              static_cast<__int128_t>(op1.sValue()) *
              static_cast<__int128_t>(
                  static_cast<unsigned __int128>(op2.uValue()));
          return VT_U(result >> 64);
#endif
        }
      }

      case ALUOp::DIVW:
      case ALUOp::DIV: {
        const VSRTL_VT_S overflow =
            (ctrl.eValue<ALUOp>() == ALUOp::DIVW) ||
                    (ctrl.eValue<ALUOp>() == ALUOp::DIV && XLEN == 32)
                ? div_overflow32
                : div_overflow64;
        if (op2.sValue() == 0) {
          return VT_U(-1);
        } else if (op1.sValue() == overflow && op2.sValue() == -1) {
          // Overflow
          return VT_U(overflow);
        } else {
          return VT_U(op1.sValue() / op2.sValue());
        }
      }

      case ALUOp::DIVUW:
      case ALUOp::DIVU: {
        if (op2.uValue() == 0) {
          return VT_U(-1LL);
        } else {
          return op1.uValue() / op2.uValue();
        }
      }

      case ALUOp::REMW:
      case ALUOp::REM: {
        const VSRTL_VT_S overflow =
            (ctrl.eValue<ALUOp>() == ALUOp::REMW) ||
                    (ctrl.eValue<ALUOp>() == ALUOp::REM && XLEN == 32)
                ? div_overflow32
                : div_overflow64;
        if (op2.sValue() == 0) {
          return op1.uValue();
        } else if (op1.sValue() == overflow && op2.sValue() == -1) {
          // Overflow
          return VT_U(0);
        } else {
          return VT_U(op1.sValue() % op2.sValue());
        }
      }

      case ALUOp::REMUW:
      case ALUOp::REMU: {
        if (op2.uValue() == 0) {
          return op1.uValue();
        } else {
          return op1.uValue() % op2.uValue();
        }
      }

      case ALUOp::AND:
        return op1.uValue() & op2.uValue();

      case ALUOp::OR:
        return op1.uValue() | op2.uValue();

      case ALUOp::XOR:
        return op1.uValue() ^ op2.uValue();

      case ALUOp::SL:
      case ALUOp::SRA:
      case ALUOp::SRL: {
        VSRTL_VT_U shiftMask;
        if constexpr (XLEN == 32) {
          shiftMask = generateBitmask(5);
        } else {
          shiftMask = generateBitmask(6);
        }
        VSRTL_VT_U shiftAmount = op2.uValue() & shiftMask;

        switch (ctrl.eValue<ALUOp>()) {
        case ALUOp::SL:
          return op1.uValue() << shiftAmount;
        case ALUOp::SRA:
          return VT_U(op1.sValue() >> shiftAmount);
        case ALUOp::SRL:
          return op1.uValue() >> shiftAmount;
        default:
          assert(false); // unreachable
        }
      }

      case ALUOp::LUI:
        return VT_U(signextend<32>(op2.uValue()));

      case ALUOp::LT:
        return VT_U(op1.sValue() < op2.sValue() ? 1 : 0);

      case ALUOp::LTU:
        return VT_U(op1.uValue() < op2.uValue() ? 1 : 0);

      case ALUOp::NOP:
        return VT_U(0xDEADBEEF);

      case ALUOp::ADDW:
        return VT_U(signextend<32>(op1.uValue() + op2.uValue()));

      case ALUOp::SUBW:
        return VT_U(signextend<32>(op1.uValue() - op2.uValue()));

      case ALUOp::SLW:
        return VT_U(signextend<32>(op1.uValue()
                                   << (op2.uValue() & generateBitmask(5))));

      case ALUOp::SRAW:
        return VT_U(signextend<32>(static_cast<int32_t>(op1.uValue()) >>
                                   (op2.uValue() & generateBitmask(5))));

      case ALUOp::SRLW:
        return VT_U(signextend<32>(static_cast<uint32_t>(op1.uValue()) >>
                                   (op2.uValue() & generateBitmask(5))));

      default:
        throw std::runtime_error("Invalid ALU opcode");
      }
    };
  }

  INPUTPORT_ENUM(ctrl, ALUOp);
  INPUTPORT(op1, XLEN);
  INPUTPORT(op2, XLEN);

  OUTPUTPORT(res, XLEN);
};


/**
 * @brief RISC-V Floating Point Control and Status Register (fcsr)
 */
class Fcsr {
public:
  static constexpr uint32_t NX_mask  = 1u << 0;
  static constexpr uint32_t UF_mask  = 1u << 1;
  static constexpr uint32_t OF_mask  = 1u << 2;
  static constexpr uint32_t DZ_mask  = 1u << 3;
  static constexpr uint32_t NV_mask  = 1u << 4;
  static constexpr uint32_t frm_mask = 0b111u << 5;
  
  struct Flags {
    bool NX; // inexact
    bool UF; // underflow
    bool OF; // overflow
    bool DZ; // divide by zero
    bool NV; // invalid operation

    inline uint32_t toWord() const {
      uint32_t word = 0;
      if (NX) word |= NX_mask;
      if (UF) word |= UF_mask;
      if (OF) word |= OF_mask;
      if (DZ) word |= DZ_mask;
      if (NV) word |= NV_mask;
      return word;
    }
    static inline Flags fromWord(uint32_t word) {
      return Flags{
        .NX = (word & NX_mask) != 0,
        .UF = (word & UF_mask) != 0,
        .OF = (word & OF_mask) != 0,
        .DZ = (word & DZ_mask) != 0,
        .NV = (word & NV_mask) != 0,
      };
    }
  };
  
  uint32_t word; // word representing the riscv fcsr register layout

  Fcsr() 
    : word(Fcsr::defaultFcsr().word) {}
  Fcsr(uint32_t val) 
    : word(val) {}
  Fcsr(Flags flags, RoundMode rm) 
    : word(0) {
    word |= flags.toWord();
    word |= (static_cast<uint32_t>(rm) << 5) & frm_mask;
  }

  Flags getFlags() const {
    return Flags::fromWord(word);
  }
  RoundMode getRoundingMode() const {
    return static_cast<RoundMode>((word & frm_mask) >> 5);
  }

  // clear all reserved bits but leave frm and fflags unchanged
  void canonicalize() {
    word &= (0b1 << 8) - 1; // only keep the lowest 8 bits of fcsr, since the rest are reserved
  }

  // write fcsr state to softfloats (thread-local) state, filters out invalid rounding modes
  void writeToSoftFloat() const {
    const Flags flags = getFlags();
    softfloat_exceptionFlags = 0;
    if (flags.NX) softfloat_exceptionFlags |= softfloat_flag_inexact;
    if (flags.UF) softfloat_exceptionFlags |= softfloat_flag_underflow;
    if (flags.OF) softfloat_exceptionFlags |= softfloat_flag_overflow;
    if (flags.DZ) softfloat_exceptionFlags |= softfloat_flag_infinite;
    if (flags.NV) softfloat_exceptionFlags |= softfloat_flag_invalid;

    const RoundMode rm = getRoundingMode();
    switch (rm) {
      case RoundMode::RNE:
        softfloat_roundingMode = softfloat_round_near_even; 
        break;
      case RoundMode::RTZ:
        softfloat_roundingMode = softfloat_round_minMag; 
        break;
      case RoundMode::RDN:
        softfloat_roundingMode = softfloat_round_min; 
        break;
      case RoundMode::RUP:
        softfloat_roundingMode = softfloat_round_max; 
        break;
      case RoundMode::RMM:
        softfloat_roundingMode = softfloat_round_near_maxMag; 
        break;
      
      case RoundMode::DYN:
      case RoundMode::_RSVED01:
      case RoundMode::_RSVED10:
        // add custom trap handler here if needed
        // use C default rounding mode
        softfloat_roundingMode = softfloat_round_near_even; 
        break;

      default: Q_UNREACHABLE();
    }
  }

  // default fcsr value after reset
  static Fcsr defaultFcsr() {
    return Fcsr( Flags::fromWord( 0 ), RoundMode::RNE );
  }

  // load fcsr state from softfloat's (thread-local) state
  static Fcsr loadFromSoftFloat() {
    Flags flags{
      (softfloat_exceptionFlags & softfloat_flag_inexact)   != 0,
      (softfloat_exceptionFlags & softfloat_flag_underflow) != 0,
      (softfloat_exceptionFlags & softfloat_flag_overflow)  != 0,
      (softfloat_exceptionFlags & softfloat_flag_infinite)  != 0,
      (softfloat_exceptionFlags & softfloat_flag_invalid)   != 0,
    };
    RoundMode rm;
    switch (softfloat_roundingMode) {
      case softfloat_round_near_even:    rm = RoundMode::RNE; break;
      case softfloat_round_minMag:       rm = RoundMode::RTZ; break;
      case softfloat_round_min:          rm = RoundMode::RDN; break;
      case softfloat_round_max:          rm = RoundMode::RUP; break;
      case softfloat_round_near_maxMag:  rm = RoundMode::RMM; break;
      
      default:
        // add custom trap handler here if needed
        // use C default rounding mode
        rm = RoundMode::RNE; break;
    }
    return Fcsr(flags, rm);
  }
};

template <unsigned XLEN>
class FPU : public Component {
public:
  class FcsrReg : public Register<32> {
    public:
    using Register<32>::Register;

    FcsrReg(const std::string &name, SimComponent *parent)
      : Register<32>(name, parent) {
        m_initvalue = VT_U(Fcsr::defaultFcsr().word);
      }
  };

protected:
  inline Fcsr getFcsrFromReg() {
    return Fcsr( static_cast<uint32_t>(fcsr->out.uValue()) );
  }

  enum class csrInstr { csrrw, csrrs, csrrc };
  VSRTL_VT_U modifyFflags(csrInstr csr, VSRTL_VT_U rs1) {
    // reads the old fcsr value before writing the new fcsr value
    // analogous implementation to csrrw instruction
    const Fcsr xfcsr = getFcsrFromReg();

    const VSRTL_VT_U oldFflags = VT_U(xfcsr.getFlags().toWord());

    rs1 = rs1 & 0b11111; // only consider the lowest 5 bits of rs1 for fflags
    Fcsr::Flags nextFflags;
    switch(csr) {
      case csrInstr::csrrw: nextFflags = Fcsr::Flags::fromWord(rs1); break;
      case csrInstr::csrrs: nextFflags = Fcsr::Flags::fromWord(oldFflags | rs1); break;
      case csrInstr::csrrc: nextFflags = Fcsr::Flags::fromWord(oldFflags & ~rs1); break;
    }

    // write updated fflags to Softfloat's fflags state
    Fcsr{ nextFflags, xfcsr.getRoundingMode() }.writeToSoftFloat();

    return oldFflags;
  }
  VSRTL_VT_U modifyFrm(csrInstr csr, VSRTL_VT_U rs1) {
    // reads the old fcsr value before writing the new fcsr value
    // analogous implementation to csrrw instruction
    const Fcsr xfcsr = getFcsrFromReg();

    const VSRTL_VT_U oldFrm = VT_U(xfcsr.getRoundingMode());

    VSRTL_VT_U nextFrm_vt_u;
    switch(csr) {
      case csrInstr::csrrw: nextFrm_vt_u = rs1; break;
      case csrInstr::csrrs: nextFrm_vt_u = oldFrm | rs1; break;
      case csrInstr::csrrc: nextFrm_vt_u = oldFrm & ~rs1; break;
    }

    // only consider the lowest 3 bits of nextFrm_vt_u for frm, since there are only 3 bits allocated for frm in fcsr
    RVISA::ExtF::RoundMode nextFrm = static_cast<RVISA::ExtF::RoundMode>(nextFrm_vt_u & 0b111);
    
    // write updated frm to Softfloat's rounding mode state
    Fcsr{ xfcsr.getFlags(), nextFrm }.writeToSoftFloat();
    
    return oldFrm;
  }
  VSRTL_VT_U modifyFcsr(csrInstr csr, VSRTL_VT_U rs1) {
    // reads the old fcsr value before writing the new fcsr value
    // analogous implementation to csrrw instruction
    const VSRTL_VT_U oldFcsr = VT_U(getFcsrFromReg().word);
    
    VSRTL_VT_U nextFcsr_vt_u;
    switch(csr) {
      case csrInstr::csrrw: nextFcsr_vt_u = rs1; break;
      case csrInstr::csrrs: nextFcsr_vt_u = oldFcsr | rs1; break;
      case csrInstr::csrrc: nextFcsr_vt_u = oldFcsr & ~rs1; break;
    }
    
    Fcsr nextFcsr{ static_cast<uint32_t>(nextFcsr_vt_u) };

    nextFcsr.canonicalize();
    nextFcsr.writeToSoftFloat(); // store updated fcsr into softfloat state

    return oldFcsr;
  }

  static uint_fast8_t mapRoundingMode(const RVISA::ExtF::RoundMode rm) {
    switch (rm) {
      case RVISA::ExtF::RoundMode::RNE: return softfloat_round_near_even;
      case RVISA::ExtF::RoundMode::RTZ: return softfloat_round_minMag;
      case RVISA::ExtF::RoundMode::RDN: return softfloat_round_min;
      case RVISA::ExtF::RoundMode::RUP: return softfloat_round_max;
      case RVISA::ExtF::RoundMode::RMM: return softfloat_round_near_maxMag;
      case RVISA::ExtF::RoundMode::DYN: return softfloat_roundingMode; // use preloaded value from fcsr
      
      case RVISA::ExtF::RoundMode::_RSVED01:
      case RVISA::ExtF::RoundMode::_RSVED10:
        // add custom trap handler here if needed
        return softfloat_round_near_even; // use C default rounding mode

      default: Q_UNREACHABLE();
    }
  }

public:
  FPU(const std::string &name, SimComponent *parent)
    : Component(name, parent) {
    
    connect_res->setSensitiveTo(ctrl);
    connect_res->setSensitiveTo(roundmode);
    connect_res->setSensitiveTo(op1);
    connect_res->setSensitiveTo(op2);
    connect_res->setSensitiveTo(op3);
    connect_res->setSensitiveTo(fcsr->out);
    connect_res->out << [this] {
      Float32_t fs1{ .word = static_cast<uint32_t>(op1.uValue()) };
      Float32_t fs2{ .word = static_cast<uint32_t>(op2.uValue()) };
      Float32_t fs3{ .word = static_cast<uint32_t>(op3.uValue()) };
      
      // restore/synchronize fcsr state to softfloat before executing the floating point operation
      Fcsr xfcsr = getFcsrFromReg();
      xfcsr.writeToSoftFloat(); 
      
      // clang-format off
      // all fpu instructions which do not use static rounding ------------------------------------
      switch (ctrl.eValue<FPUOp>()) {
        case FPUOp::NOP:
          // being fancy so that we can differentiate invalid float Data from invalid integer Data
          return VT_U( 0xDEADDAB );
        
        case FPUOp::FMIN_S:       return VT_U(Float32_t::min(fs1, fs2).word);
        case FPUOp::FMAX_S:       return VT_U(Float32_t::max(fs1, fs2).word);

        case FPUOp::FSGNJ_S:      return VT_U(fs1.setSign( fs2.sign()).word);
        case FPUOp::FSGNJN_S:     return VT_U(fs1.setSign(~fs2.sign()).word);
        case FPUOp::FSGNJX_S:     return VT_U(fs1.setSign( fs1.sign() ^ fs2.sign()).word);
        
        case FPUOp::FMV_X_W: Q_FALLTHROUGH();
        case FPUOp::FMV_W_X:
          return op1.uValue();
        
        case FPUOp::EQ:           return VT_U( fs1 == fs2 ? 1 : 0 );
        case FPUOp::LE:           return VT_U( fs1 <= fs2 ? 1 : 0 );
        case FPUOp::LT:           return VT_U( fs1 <  fs2 ? 1 : 0 );
        
        case FPUOp::FCLASS_S:
          // shift to match RISC-V fclass.s encoding
          return VT_U( 1 << static_cast<uint32_t>(fs1.fclass()) );
        
        // handle Zicsr instructions that modify the float CSRs
        case FPUOp::CSRW_FFLAGS:  return modifyFflags(csrInstr::csrrw, op1.uValue());
        case FPUOp::CSRS_FFLAGS:  return modifyFflags(csrInstr::csrrs, op1.uValue());
        case FPUOp::CSRC_FFLAGS:  return modifyFflags(csrInstr::csrrc, op1.uValue());
        
        case FPUOp::CSRW_FRM:     return modifyFrm(csrInstr::csrrw, op1.uValue());
        case FPUOp::CSRS_FRM:     return modifyFrm(csrInstr::csrrs, op1.uValue());
        case FPUOp::CSRC_FRM:     return modifyFrm(csrInstr::csrrc, op1.uValue());

        case FPUOp::CSRW_FCSR:    return modifyFcsr(csrInstr::csrrw, op1.uValue());
        case FPUOp::CSRS_FCSR:    return modifyFcsr(csrInstr::csrrs, op1.uValue());
        case FPUOp::CSRC_FCSR:    return modifyFcsr(csrInstr::csrrc, op1.uValue());

        default:
          break; // for all other instructions, we will apply static rounding based on the roundmode input port
      }

      // all remaining instructions which do use static rounding ----------------------------------
      softfloat_roundingMode = mapRoundingMode(roundmode.eValue<RVISA::ExtF::RoundMode>());
      
      VSRTL_VT_U resValue;
      switch (ctrl.eValue<FPUOp>()) {
        case FPUOp::FADD_S:     resValue = VT_U((fs1 + fs2).word); break;
        case FPUOp::FSUB_S:     resValue = VT_U((fs1 - fs2).word); break;
        case FPUOp::FMUL_S:     resValue = VT_U((fs1 * fs2).word); break;
        case FPUOp::FDIV_S:     resValue = VT_U((fs1 / fs2).word); break;
        case FPUOp::FSQRT_S:    resValue = VT_U(fs1.sqrt().word); break;
        
        case FPUOp::FMADD_S:    resValue = VT_U(Float32_t::fma(fs1, fs2, fs3, Float32_t::fma_mode::add_AB_add_C).word); break;
        case FPUOp::FMSUB_S:    resValue = VT_U(Float32_t::fma(fs1, fs2, fs3, Float32_t::fma_mode::add_AB_sub_C).word); break;
        case FPUOp::FNMSUB_S:   resValue = VT_U(Float32_t::fma(fs1, fs2, fs3, Float32_t::fma_mode::sub_AB_add_C).word); break;
        case FPUOp::FNMADD_S:   resValue = VT_U(Float32_t::fma(fs1, fs2, fs3, Float32_t::fma_mode::add_AB_add_C).negate().word); break;
        
        case FPUOp::FCVT_W_S:   resValue = VT_U(Float32_t::to<int32_t>( fs1 )); break;
        case FPUOp::FCVT_WU_S:  resValue = VT_U(Float32_t::to<uint32_t>( fs1 )); break;
        case FPUOp::FCVT_S_W:   resValue = VT_U(Float32_t::from<int32_t>( op1.sValue() ).word); break;
        case FPUOp::FCVT_S_WU:  resValue = VT_U(Float32_t::from<uint32_t>( op1.uValue() ).word); break;

        case FPUOp::FCVT_L_S:   resValue = VT_U(Float32_t::to<int64_t>( fs1 )); break;
        case FPUOp::FCVT_LU_S:  resValue = VT_U(Float32_t::to<uint64_t>( fs1 )); break;
        case FPUOp::FCVT_S_L:   resValue = VT_U(Float32_t::from<int64_t>( op1.sValue() ).word); break;
        case FPUOp::FCVT_S_LU:  resValue = VT_U(Float32_t::from<uint64_t>( op1.uValue() ).word); break;
        
        default:
          throw std::runtime_error("Invalid / Unknown FPU opcode");
      }
      // clang-format on

      // restore the actual rounding mode from before applying static rounding
      softfloat_roundingMode = mapRoundingMode(xfcsr.getRoundingMode());

      return resValue;
    };
    connect_res->out >> res;
    
    connect_reg->setSensitiveTo(connect_res->out);
    connect_reg->out << [this] { return VT_U( Fcsr::loadFromSoftFloat().word ); };
    connect_reg->out >> fcsr->in;
  }

  SUBCOMPONENT(fcsr, FcsrReg);
  WIRE(connect_reg, 32);
  WIRE(connect_res, XLEN);

  INPUTPORT_ENUM(ctrl, FPUOp);
  INPUTPORT_ENUM(roundmode, RVISA::ExtF::RoundMode);
  
  INPUTPORT(op1, XLEN);
  INPUTPORT(op2, XLEN);
  INPUTPORT(op3, XLEN);

  OUTPUTPORT(res, XLEN);
};

namespace displayFlag {
  enum class FcsrDisplayFlag : uint8_t { O = 0, T = 1 };
} // namespace displayFlag

template <unsigned XLEN>
class FPU_Fcsr : public FPU<XLEN> {
public:
  FPU_Fcsr(const std::string &name, SimComponent *parent)
    : FPU<XLEN>(name, parent) {
    inexact << [this] { return this->getFcsrFromReg().getFlags().NX; };
    underflow << [this] { return this->getFcsrFromReg().getFlags().UF; };
    overflow << [this] { return this->getFcsrFromReg().getFlags().OF; };
    divide_by_zero << [this] { return this->getFcsrFromReg().getFlags().DZ; };
    invalid_operation << [this] { return this->getFcsrFromReg().getFlags().NV; };
    frm << [this] { return VT_U(this->getFcsrFromReg().getRoundingMode()); };
  }

  // pure display output ports
  // fflags bits: NX UF OF DZ NV
  OUTPUTPORT_ENUM(inexact, displayFlag::FcsrDisplayFlag);
  OUTPUTPORT_ENUM(underflow, displayFlag::FcsrDisplayFlag);
  OUTPUTPORT_ENUM(overflow, displayFlag::FcsrDisplayFlag);
  OUTPUTPORT_ENUM(divide_by_zero, displayFlag::FcsrDisplayFlag);
  OUTPUTPORT_ENUM(invalid_operation, displayFlag::FcsrDisplayFlag);

  // current rounding mode
  OUTPUTPORT_ENUM(frm, RVISA::ExtF::RoundMode);
};

} // namespace core
} // namespace vsrtl
