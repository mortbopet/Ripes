#pragma once

#include "limits.h"
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
        const auto result = static_cast<int64_t>(op1.sValue()) *
                            static_cast<int64_t>(op2.sValue());
        return VT_U(result >> 32);
      }
      case ALUOp::MULHU: {
        const auto result = static_cast<uint64_t>(op1.uValue()) *
                            static_cast<uint64_t>(op2.uValue());
        return VT_U(result >> 32);
      }
      case ALUOp::MULHSU: {
        const int64_t result = static_cast<int64_t>(op1.sValue()) *
                               static_cast<uint64_t>(op2.uValue());
        return VT_U(result >> 32);
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
        return op1.uValue() << op2.uValue();

      case ALUOp::SRA:
        return VT_U(op1.sValue() >> op2.uValue());

      case ALUOp::SRL:
        return op1.uValue() >> op2.uValue();

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
    word &= 0b1 << 7;
  }

  // write fcsr state to softfloats (thread-local) state
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

  inline Fcsr getFcsrFromReg() {
    return Fcsr( static_cast<uint32_t>(fcsr->out.uValue()) );
  }

  inline VSRTL_VT_U readWriteFflags(const VSRTL_VT_U newFflags) {
    // reads the old fcsr value before writing the new fcsr value
    // analogous implementation to csrrw instruction
    Fcsr xfcsr = getFcsrFromReg();

    VSRTL_VT_U oldFflags = VT_U(xfcsr.getFlags().toWord());
    Fcsr::Flags nextFflags = Fcsr::Flags::fromWord( newFflags );

    xfcsr = Fcsr{ nextFflags, xfcsr.getRoundingMode() };
    xfcsr.writeToSoftFloat(); // store updated fflags into softfloat state

    return oldFflags;
  }
  inline VSRTL_VT_U readWriteFrm(const VSRTL_VT_U newFrm) {
    // reads the old fcsr value before writing the new fcsr value
    // analogous implementation to csrrw instruction
    Fcsr xfcsr = getFcsrFromReg();

    VSRTL_VT_U oldFrm = VT_U(xfcsr.getRoundingMode());
    RVISA::ExtF::RoundMode nextFrm = static_cast<RVISA::ExtF::RoundMode>(newFrm);

    switch(nextFrm) {
      case RVISA::ExtF::RoundMode::RNE:
      case RVISA::ExtF::RoundMode::RTZ:
      case RVISA::ExtF::RoundMode::RDN:
      case RVISA::ExtF::RoundMode::RUP:
      case RVISA::ExtF::RoundMode::RMM:
        // valid rounding mode
        xfcsr = Fcsr{ xfcsr.getFlags(), nextFrm };
        break;

      case RVISA::ExtF::RoundMode::DYN:
      case RVISA::ExtF::RoundMode::_RSVED01:
      case RVISA::ExtF::RoundMode::_RSVED10:
        // invalid rounding mode
        // add custom trap handler here if needed
        break;

      default: Q_UNREACHABLE();
    }

    xfcsr.writeToSoftFloat(); // store updated fcsr into softfloat state

    return oldFrm;
  }
  inline VSRTL_VT_U readWriteFcsr(const VSRTL_VT_U newFcsr) {
    // reads the old fcsr value before writing the new fcsr value
    // analogous implementation to csrrw instruction
    Fcsr xfcsr = getFcsrFromReg();

    VSRTL_VT_U oldFcsr = VT_U(xfcsr.word);
    Fcsr nextFcsr{ static_cast<uint32_t>(newFcsr) };

    (void) readWriteFrm(VT_U(nextFcsr.getRoundingMode()));
    (void) readWriteFflags(VT_U(nextFcsr.getFlags().toWord()));

    xfcsr.canonicalize();
    xfcsr.writeToSoftFloat(); // store updated fcsr into softfloat state

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
      
      Fcsr xfcsr = getFcsrFromReg();
      xfcsr.writeToSoftFloat();
      
      softfloat_roundingMode = mapRoundingMode(roundmode.eValue<RVISA::ExtF::RoundMode>());

      // being fancy so that we can differentiate invalid float Data from invalid integer Data
      VSRTL_VT_U resValue = VT_U( 0xDEADDAB );
      
      // clang-format off
      switch (ctrl.eValue<FPUOp>()) {
        case FPUOp::NOP: break;
        
        case FPUOp::FADD_S:     resValue = VT_U((fs1 + fs2).word); break;
        case FPUOp::FSUB_S:     resValue = VT_U((fs1 - fs2).word); break;
        case FPUOp::FMUL_S:     resValue = VT_U((fs1 * fs2).word); break;
        case FPUOp::FDIV_S:     resValue = VT_U((fs1 / fs2).word); break;
        case FPUOp::FSQRT_S:    resValue = VT_U(fs1.sqrt().word); break;
        case FPUOp::FMIN_S:     resValue = VT_U(Float32_t::min(fs1, fs2).word); break;
        case FPUOp::FMAX_S:     resValue = VT_U(Float32_t::max(fs1, fs2).word); break;

        case FPUOp::FMADD_S:    resValue = VT_U(Float32_t::fma(fs1, fs2, fs3, Float32_t::fma_mode::add_AB_add_C).word); break;
        case FPUOp::FMSUB_S:    resValue = VT_U(Float32_t::fma(fs1, fs2, fs3, Float32_t::fma_mode::add_AB_sub_C).word); break;
        case FPUOp::FNMSUB_S:   resValue = VT_U(Float32_t::fma(fs1, fs2, fs3, Float32_t::fma_mode::sub_AB_add_C).word); break;
        case FPUOp::FNMADD_S:   resValue = VT_U(Float32_t::fma(fs1, fs2, fs3, Float32_t::fma_mode::add_AB_add_C).negate().word); break;
        
        case FPUOp::FSGNJ_S:    resValue = VT_U(fs1.setSign( fs2.sign()).word); break;
        case FPUOp::FSGNJN_S:   resValue = VT_U(fs1.setSign(~fs2.sign()).word); break;
        case FPUOp::FSGNJX_S:   resValue = VT_U(fs1.setSign( fs1.sign() ^ fs2.sign()).word); break;
        
        case FPUOp::FCVT_W_S:   resValue = VT_U(Float32_t::to<int32_t>( fs1 )); break;
        case FPUOp::FCVT_WU_S:  resValue = VT_U(Float32_t::to<uint32_t>( fs1 )); break;
        case FPUOp::FCVT_S_W:   resValue = VT_U(Float32_t::from<int32_t>( op1.sValue() ).word); break;
        case FPUOp::FCVT_S_WU:  resValue = VT_U(Float32_t::from<uint32_t>( op1.uValue() ).word); break;

        case FPUOp::FCVT_L_S:   resValue = VT_U(Float32_t::to<int64_t>( fs1 )); break;
        case FPUOp::FCVT_LU_S:  resValue = VT_U(Float32_t::to<uint64_t>( fs1 )); break;
        case FPUOp::FCVT_S_L:   resValue = VT_U(Float32_t::from<int64_t>( op1.sValue() ).word); break;
        case FPUOp::FCVT_S_LU:  resValue = VT_U(Float32_t::from<uint64_t>( op1.uValue() ).word); break;
        
        case FPUOp::FMV_X_W: Q_FALLTHROUGH();
        case FPUOp::FMV_W_X:
          resValue = op1.uValue(); break;
        
        case FPUOp::EQ:         resValue = VT_U( fs1 == fs2 ? 1 : 0 ); break;
        case FPUOp::LE:         resValue = VT_U( fs1 <= fs2 ? 1 : 0 ); break;
        case FPUOp::LT:         resValue = VT_U( fs1 <  fs2 ? 1 : 0 ); break;
        
        // shift to match RISC-V fclass.s encoding
        case FPUOp::FCLASS_S:   resValue = VT_U( 1 << static_cast<uint32_t>(fs1.fclass()) ); break;
        
        // since fflags have been already synced after the previus operation,
        // we can just return the current fcsr value 
        case FPUOp::FRCSR:      resValue = VT_U(xfcsr.word); break;
        case FPUOp::FRRM:       resValue = VT_U(xfcsr.getRoundingMode()); break;
        case FPUOp::FRFLAGS:    resValue = VT_U(xfcsr.getFlags().toWord()); break;

        // operations that modify the frm field must skip over the restoration of the rounding mode 
        // at the end of this lambda since they already overwrite the rounding mode
        case FPUOp::FSCSR:      resValue = readWriteFcsr(op1.uValue()); goto skip_roundind_mode_restore;
        case FPUOp::FSRM:       resValue = readWriteFrm(op1.uValue()); goto skip_roundind_mode_restore;

        case FPUOp::FSFLAGS:    resValue = readWriteFflags(op1.uValue()); break;

        default:
          throw std::runtime_error("Invalid FPU opcode");
      }
      // clang-format on

      // restore true rounding mode from the temporarily applied static rounding mode
      softfloat_roundingMode = mapRoundingMode(xfcsr.getRoundingMode());

      skip_roundind_mode_restore:
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

template <unsigned XLEN>
class FPU_Fcsr : public FPU<XLEN> {
public:
  enum class DisplayFlag { O=0, T=1 };

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
  OUTPUTPORT_ENUM(inexact, DisplayFlag);
  OUTPUTPORT_ENUM(underflow, DisplayFlag);
  OUTPUTPORT_ENUM(overflow, DisplayFlag);
  OUTPUTPORT_ENUM(divide_by_zero, DisplayFlag);
  OUTPUTPORT_ENUM(invalid_operation, DisplayFlag);

  // current rounding mode
  OUTPUTPORT_ENUM(frm, RVISA::ExtF::RoundMode);
};

} // namespace core
} // namespace vsrtl
