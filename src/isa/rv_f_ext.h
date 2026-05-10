#pragma once

#include <bitset>
#include <iostream>

#include "pseudoinstruction.h"
#include "rvisainfo_common.h"

// clang-format off
/**
 * Float instruction register mapping
 * Instruction  parameters                Description
 * flw          fd, imm(rs1)              fd = M[rs1 + imm]
 * fsw          fs2, imm(rs1)             M[rs1 + imm] = fs2
 * 
 * fmadd.s      fd, fs1, fs2, fs3 [, rm]  fd = fs1 * fs2 + fs3
 * fmsub.s      fd, fs1, fs2, fs3 [, rm]  fd = fs1 * fs2 - fs3
 * fnmadd.s     fd, fs1, fs2, fs3 [, rm]  fd = -(fs1 * fs2) - fs3
 * fnmsub.s     fd, fs1, fs2, fs3 [, rm]  fd = -(fs1 * fs2) + fs3
 * 
 * fadd.s       fd, fs1, fs2 [, rm]       fd = fs1 + fs2
 * fsub.s       fd, fs1, fs2 [, rm]       fd = fs1 - fs2
 * fmul.s       fd, fs1, fs2 [, rm]       fd = fs1 * fs2
 * fdiv.s       fd, fs1, fs2 [, rm]       fd = fs1 / fs2
 * fsqrt.s      fd, fs1 [, rm]            fd = sqrt(fs1)
 * fmin.s       fd, fs1, fs2              fd = min(fs1, fs2)
 * fmax.s       fd, fs1, fs2              fd = max(fs1, fs2)
 * 
 * fsgnj.s      fd, fs1, fs2              fd = abs(fs1) * sgn(fs2)
 * fsgnjn.s     fd, fs1, fs2              fd = abs(fs1) * -sgn(fs2)
 * fsgnjx.s     fd, fs1, fs2              fd = fs1 * sgn(fs2)
 * 
 * fcvt.s.w     fd, rs1 [, rm]            fd = (float) rs1
 * fcvt.s.wu    fd, rs1 [, rm]            fd = (float) rs1
 * fcvt.w.s     rd, fs1 [, rm]            rd = (int32_t) fs1
 * fcvt.wu.s    rd, fs1 [, rm]            rd = (uint32_t) fs1
 * fcvt.s.l     fd, rs1 [, rm]            fd = (float) rs1
 * fcvt.s.lu    fd, rs1 [, rm]            fd = (float) rs1
 * fcvt.l.s     rd, fs1 [, rm]            rd = (int64_t) fs1
 * fcvt.lu.s    rd, fs1 [, rm]            rd = (uint64_t) fs1
 * 
 * fmv.x.w      rd, fs1                   rd = *((int*) &fs1)
 * fmv.w.x      fd, rs1                   fd = *((float*) &rs1)
 * 
 * feq.s        rd, fs1, fs2              rd = (fs1 == fs2) ? 1 : 0
 * flt.s        rd, fs1, fs2              rd = (fs1 <  fs2) ? 1 : 0
 * fle.s        rd, fs1, fs2              rd = (fs1 <= fs2) ? 1 : 0
 * 
 * fclass.s     rd, fs1                   rd = fclass(fs1)
 * 
 * Pseudo Instructions
 * fmv.s        fd, fs1                   fd = fs1
 * fabs.s       fd, fs1                   fd = |fs1|
 * fneg.s       fd, fs1                   fd = -fs1
 * 
 * pseudo CSR instructions for floating-point control and status register:
 * frcsr        rd                        rd = fcsr                 >> expands to: CSRRS rd, fcsr, x0
 * fscsr        rd, rs1                   rd = fcsr; fcsr = rs1     >> expands to: CSRRW rd, fcsr, rs1
 * frrm         rd                        rd = frm                  >> expands to: CSRRS rd, frm, x0
 * fsrm         rd, rs1                   rd = frm; frm = rs1       >> expands to: CSRRW rd, frm, rs1
 * frflags      rd                        rd = fflags               >> expands to: CSRRS rd, fflags, x0
 * fsflags      rd, rs1                   rd = fflags; fflags = rs1 >> expands to: CSRRW rd, fflags, rs1
 */

namespace Ripes {
namespace RVISA {
namespace ExtF {
  /// The RISC-V Rs1 field contains a float source register index.
  /// It is defined as a 5-bit field in bits 15-19 of the instruction
  template <unsigned tokenIndex>
  struct FRegRs1
      : public FPR_Reg<FRegRs1<tokenIndex>, tokenIndex, BitRange<15, 19>> {
    constexpr static std::string_view getName() { return "fs1"; }
  };

  /// The RISC-V Rs2 field contains a float source register index.
  /// It is defined as a 5-bit field in bits 20-24 of the instruction
  template <unsigned tokenIndex>
  struct FRegRs2
      : public FPR_Reg<FRegRs2<tokenIndex>, tokenIndex, BitRange<20, 24>> {
    constexpr static std::string_view getName() { return "fs2"; }
  };

  /// The RISC-V Rs2 field contains a float source register index.
  /// It is defined as a 5-bit field in bits 20-24 of the instruction
  template <unsigned tokenIndex>
  struct FRegRs3
      : public FPR_Reg<FRegRs2<tokenIndex>, tokenIndex, BitRange<27, 31>> {
    constexpr static std::string_view getName() { return "fs3"; }
  };

  /// The RISC-V Rd field contains a float destination register index.
  /// It is defined as a 5-bit field in bits 7-11 of the instruction
  template <unsigned tokenIndex>
  struct FRegRd : public FPR_Reg<FRegRd<tokenIndex>, tokenIndex, BitRange<7, 11>> {
    constexpr static std::string_view getName() { return "fd"; }
  };

  enum class Width : unsigned {
    W = 0b010,  // Word
    D = 0b011,  // Double
    H = 0b001,  // Half
    Q = 0b100   // Quad
  };
  template <Width width, unsigned N = 32>
  struct OpPartWidth : public OpPart<static_cast<unsigned>(width), BitRange<12, 14, N>> {};

  typedef unsigned funct3_t;
  enum class RoundMode : funct3_t {
    RNE = 0b000, // round to nearest
    RTZ = 0b001, // round towards zero
    RDN = 0b010, // round down (towards -inf)
    RUP = 0b011, // round up   (towards +inf)
    RMM = 0b100, // round to nearest, ties to max magnitude
    _RSVED01 = 0b101, // reserved
    _RSVED10 = 0b110, // reserved
    DYN = 0b111 // dynamic rounding
  };
  template <auto funct3, unsigned N = 32>
  struct OpPartFunct3 : public OpPart<static_cast<unsigned>(funct3), BitRange<12, 14, N>> {};

  /**
   * @brief Immediate for optional Rounding mode argument in float instructions.
   *        Missing or empty immediate will default to DYN rounding mode.
   * @param tokenIndex: Index within a list of decoded instruction tokens that
   * corresponds to the immediate
   */
  template <unsigned tokenIndex, typename ImmParts=ImmPart<0, 12, 14>>
  struct ImmRM : public Field<tokenIndex, typename ImmParts::BitRanges> {
    static constexpr unsigned width = 3;

    using Reg_T_S = typename std::make_signed<Reg_T>::type;
    using Reg_T_U = typename std::make_unsigned<Reg_T>::type;

    /// Converts a string to its immediate value (if it exists). Success is set
    /// to false if this fails.
    static int64_t getImm(const QString &immToken, bool &success,
                    ImmConvInfo &convInfo) {
      // check for round mode aliases
      success = true;
      const QString immLower = immToken.toLower();

      if (immLower == "rne") return static_cast<int64_t>(RoundMode::RNE);
      if (immLower == "rtz") return static_cast<int64_t>(RoundMode::RTZ);
      if (immLower == "rdn") return static_cast<int64_t>(RoundMode::RDN);
      if (immLower == "rup") return static_cast<int64_t>(RoundMode::RUP);
      if (immLower == "rmm") return static_cast<int64_t>(RoundMode::RMM);
      if (immLower == "dyn") return static_cast<int64_t>(RoundMode::DYN);
      if (immLower == "")    return static_cast<int64_t>(RoundMode::DYN);

      // else parse as conventional immediate
      success = false;
      return getImmediate(immToken, success, &convInfo);
    }

    /// Returns an error if `value` does not fit in this immediate.
    static Result<> checkFitsInWidth(Reg_T_S value, const Location &sourceLine,
                                    ImmConvInfo&, QString token = QString()) {
      if (!isUInt(width, value)) {
        if (token.isEmpty())
          token = QString::number(static_cast<Reg_T_U>(value));
        
        return Error(
          sourceLine, 
          "Immediate rounding mode value '" + token + "' does not fit in 3 bits"
        );
      }

      // reserved roundmodes are invalid and should fail
      if (value == static_cast<int64_t>(RoundMode::_RSVED01) 
       || value == static_cast<int64_t>(RoundMode::_RSVED10)) {
        return Error(
          sourceLine, 
          "Immediate rounding mode value '" + token + "' is a reserved rounding mode"
        );
      }

      return Result<>::def();
    }

    /// Symbol resolver function for this immediate.
    static Result<> applySymbolResolution(const Location &loc, Reg_T symbolValue,
                                          Instr_T &instruction, Reg_T) {
      ImmConvInfo convInfo;
      convInfo.radix = Radix::Unsigned;

      if (auto res = checkFitsInWidth(symbolValue, loc, convInfo);
          res.isError())
        return res.error();

      ImmParts::apply(symbolValue, instruction);

      return Result<>::def();
    }

    /// Applies this immediate's encoding to the instruction.
    static Result<> apply(const TokenizedSrcLine &line, Instr_T &instruction,
                          FieldLinkRequest &linksWithSymbol) {
      bool success = false;
      ImmConvInfo convInfo;
      
      const Token &immToken = line.tokens.value(tokenIndex + 1, QString("") /* empty for default rounding mode */);
      Reg_T_S value = getImm(immToken, success, convInfo);

      if (!success) {
        // Could not directly resolve immediate. Register it as a symbol to link
        // to.
        linksWithSymbol.resolveSymbol = applySymbolResolution;
        linksWithSymbol.symbol = immToken;
        linksWithSymbol.relocation = immToken.relocation();
        return Result<>::def();
      }

      if (auto res = checkFitsInWidth(value, line, convInfo, immToken);
          res.isError())
        return res.error();

      ImmParts::apply(value, instruction);
      return Result<>::def();
    }
    
    /// Decodes this immediate part into its round mode value, adding it to the assembly
    /// line.
    static bool decode(const Instr_T instruction, const Reg_T,
                  const ReverseSymbolMap&,
                  LineTokens &line) {
      Instr_T reconstructed = 0;
      ImmParts::decode(reconstructed, instruction);
      
      QString roundMode;
      switch (static_cast<RoundMode>(reconstructed)) {
        case RoundMode::RNE: roundMode = "rne"; break;
        case RoundMode::RTZ: roundMode = "rtz"; break;
        case RoundMode::RDN: roundMode = "rdn"; break;
        case RoundMode::RUP: roundMode = "rup"; break;
        case RoundMode::RMM: roundMode = "rmm"; break;
        case RoundMode::DYN: roundMode = ""; break; // dynamic rounding is default and can be omitted
        default: roundMode = "invalid"; break;
      }

      line.push_back(roundMode);

      return true;
    }
  };
  
  enum class FMT : unsigned {
    S = 0b00, // single precision
    D = 0b01, // double precision
    H = 0b10, // half precision
    Q = 0b11  // quad precision
  };
  template <FMT fmt, unsigned N = 32>
  struct OpPartFmt : public OpPart<static_cast<unsigned>(fmt), BitRange<25, 26, N>> {};


  namespace TypeI {
    template <unsigned tokenIndex>
    struct ImmCommon12
        : public Imm<tokenIndex, 12, Repr::Signed, ImmPart<0, 20, 31>> {};

    template <typename InstrImpl, Width width>
    struct Instr : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::LOAD_FP>,
                             OpPartWidth<width>> {};
      struct Fields : public FieldSet<FRegRd, ImmCommon12, RegRs1> {};
    };

    struct Flw : public Instr<Flw, Width::W> {
      constexpr static std::string_view NAME = "flw";
    };
  } // namespace TypeI

  namespace TypeS {
    /// A RISC-V signed immediate field with an input width of 12 bits.
    /// Used in S-Type instructions.
    ///
    /// It is defined as:
    ///  - Imm[31:11] = Inst[31]
    ///  - Imm[10:5]  = Inst[30:25]
    ///  - Imm[4:0]   = Inst[11:7]
    constexpr static unsigned VALID_INDEX = 1;
    template <unsigned index>
    struct ImmS : public Imm<index, 12, Repr::Signed,
                            ImmPartSet<ImmPart<5, 25, 31>, ImmPart<0, 7, 11>>> {
      static_assert(index == VALID_INDEX, "Invalid token index");
    };

    template <typename InstrImpl, Width width>
    struct Instr : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::STORE_FP>,
                             OpPartWidth<width>> {};
      struct Fields : public FieldSet<FRegRs2, ImmS, RegRs1> {};
    };

    struct Fsw : public Instr<Fsw, Width::W> {
      constexpr static std::string_view NAME = "fsw";
    };
  } // namespace TypeS

  namespace TypeR {
    enum class Funct5 : unsigned {
      FADD     = 0b00000,
      FSUB     = 0b00001,
      FMUL     = 0b00010,
      FDIV     = 0b00011,
      FSQRT    = 0b01011,
      FMIN_MAX = 0b00101,

      FSGNJ    = 0b00100,

      FCVT_FMT_INT = 0b11010,
      FCVT_INT_FMT = 0b11000,

      FMV_X_W = 0b11100, // same as FCLASS
      FMV_W_X = 0b11110,

      FCMP = 0b10100,

      FCLASS = 0b11100 // same as FMV_X_W
    };

    static constexpr funct3_t f3_MIN = 0b000;
    static constexpr funct3_t f3_MAX = 0b001;

    template <Funct5 funct5, unsigned N = 32>
    struct OpPartFunct5 : public OpPart<static_cast<unsigned>(funct5), BitRange<27, 31, N>> {};

    template <typename InstrImpl, auto funct3, FMT fmt, Funct5 funct5, template<unsigned> class rd=FRegRd, template<unsigned> class rs1=FRegRs1>
    struct InstrOp3 : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::OP_FP>,
                             OpPartFunct3<funct3>,
                             OpPartFmt<fmt>,
                             OpPartFunct5<funct5>> {};
      struct Fields : public FieldSet<rd, rs1, FRegRs2> {};
    };
    
    template <typename InstrImpl, FMT fmt, Funct5 funct5, template<unsigned> class rd=FRegRd, template<unsigned> class rs1=FRegRs1>
    struct InstrRM : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::OP_FP>,
                             OpPartFmt<fmt>,
                             OpPartFunct5<funct5>> {};
      struct Fields : public FieldSet<rd, rs1, FRegRs2, ImmRM> {};
    };

    template <typename InstrImpl, auto funct3, FMT fmt, Funct5 funct5, auto rs2, template<unsigned> class rd=FRegRd, template<unsigned> class rs1=FRegRs1>
    struct InstrRs2Op : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::OP_FP>,
                             OpPartFunct3<funct3>,
                             OpPartFmt<fmt>,
                             OpPart<static_cast<unsigned>(rs2), BitRange<20, 24>>,
                             OpPartFunct5<funct5>> {};
      struct Fields : public FieldSet<rd, rs1> {};
    };

    template <typename InstrImpl, FMT fmt, Funct5 funct5, auto rs2, template<unsigned> class rd=FRegRd, template<unsigned> class rs1=FRegRs1>
    struct InstrRs2RM : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::OP_FP>,
                             OpPartFmt<fmt>,
                             OpPart<static_cast<unsigned>(rs2), BitRange<20, 24>>,
                             OpPartFunct5<funct5>> {};
      struct Fields : public FieldSet<rd, rs1, ImmRM> {};
    };


    //-------------------------------------------------------------------------
    // Float Arithmetic
    //-------------------------------------------------------------------------

    struct Fadd {
      struct s : public InstrRM<s, FMT::S, Funct5::FADD> {
        constexpr static std::string_view NAME = "fadd.s";
      };
    };
    struct Fsub {
      struct s : public InstrRM<s, FMT::S, Funct5::FSUB> {
        constexpr static std::string_view NAME = "fsub.s";
      };
    };
    struct Fmul {
      struct s : public InstrRM<s, FMT::S, Funct5::FMUL> {
        constexpr static std::string_view NAME = "fmul.s";
      };
    };
    struct Fdiv {
      struct s : public InstrRM<s, FMT::S, Funct5::FDIV> {
        constexpr static std::string_view NAME = "fdiv.s";
      };
    };

    struct Fsqrt {
      struct s : public InstrRs2RM<s, FMT::S, Funct5::FSQRT, 0> {
        constexpr static std::string_view NAME = "fsqrt.s";
      };
    };

    struct Fmin {
      struct s : public InstrOp3<s, f3_MIN, FMT::S, Funct5::FMIN_MAX> {
        constexpr static std::string_view NAME = "fmin.s";
      };
    };
    struct Fmax {
      struct s : public InstrOp3<s, f3_MAX, FMT::S, Funct5::FMIN_MAX> {
        constexpr static std::string_view NAME = "fmax.s";
      };
    };


    //-------------------------------------------------------------------------
    // Float Sign-Injection
    //-------------------------------------------------------------------------
    
    enum class SgnJ : funct3_t {
      FSGNJ  = 0b000,
      FSGNJN = 0b001,
      FSGNJX = 0b010
    };

    struct Fsgnj {
      struct s : public InstrOp3<s, SgnJ::FSGNJ, FMT::S, Funct5::FSGNJ> {
        constexpr static std::string_view NAME = "fsgnj.s";
      };
    };
    struct Fsgnjn {
      struct s : public InstrOp3<s, SgnJ::FSGNJN, FMT::S, Funct5::FSGNJ> {
        constexpr static std::string_view NAME = "fsgnjn.s";
      };
    };
    struct Fsgnjx {
      struct s : public InstrOp3<s, SgnJ::FSGNJX, FMT::S, Funct5::FSGNJ> {
        constexpr static std::string_view NAME = "fsgnjx.s";
      };
    };


    //-------------------------------------------------------------------------
    // Float Conversions
    //-------------------------------------------------------------------------

    enum class Cvt_Int : unsigned {
      W  = 0b00000,
      WU = 0b00001,
      L  = 0b00010,
      LU = 0b00011
    };

    template <typename InstrImpl, FMT fmt, Cvt_Int cvt>
    using InstrOp_Int_Fmt = InstrRs2Op<InstrImpl, RoundMode::DYN, fmt, Funct5::FCVT_INT_FMT, cvt, RegRd, FRegRs1>;
    template <typename InstrImpl, FMT fmt, Cvt_Int cvt>
    using InstrRM_Int_Fmt = InstrRs2RM<InstrImpl, fmt, Funct5::FCVT_INT_FMT, cvt, RegRd, FRegRs1>;
    
    template <typename InstrImpl, FMT fmt, Cvt_Int cvt>
    using InstrOp_Fmt_Int = InstrRs2Op<InstrImpl, RoundMode::DYN, fmt, Funct5::FCVT_FMT_INT, cvt, FRegRd, RegRs1>;
    template <typename InstrImpl, FMT fmt, Cvt_Int cvt>
    using InstrRM_Fmt_Int = InstrRs2RM<InstrImpl, fmt, Funct5::FCVT_FMT_INT, cvt, FRegRd, RegRs1>;

    struct Fcvt {
      struct w {
        struct s : public InstrRM_Int_Fmt<s, FMT::S, Cvt_Int::W> {
          constexpr static std::string_view NAME = "fcvt.w.s";
        };
      };
      
      struct wu {
        struct s : public InstrRM_Int_Fmt<s, FMT::S, Cvt_Int::WU> {
          constexpr static std::string_view NAME = "fcvt.wu.s";
        };
      };
      
      struct l {
        struct s : public InstrRM_Int_Fmt<s, FMT::S, Cvt_Int::L> {
          constexpr static std::string_view NAME = "fcvt.l.s";
        };
      };
      
      struct lu {
        struct s : public InstrRM_Int_Fmt<s, FMT::S, Cvt_Int::LU> {
          constexpr static std::string_view NAME = "fcvt.lu.s";
        };
      };

      struct s {
        struct w : public InstrRM_Fmt_Int<w, FMT::S, Cvt_Int::W> {
          constexpr static std::string_view NAME = "fcvt.s.w";
        };
        struct wu : public InstrRM_Fmt_Int<wu, FMT::S, Cvt_Int::WU> {
          constexpr static std::string_view NAME = "fcvt.s.wu";
        };
        struct l : public InstrRM_Fmt_Int<l, FMT::S, Cvt_Int::L> {
          constexpr static std::string_view NAME = "fcvt.s.l";
        };
        struct lu : public InstrRM_Fmt_Int<lu, FMT::S, Cvt_Int::LU> {
          constexpr static std::string_view NAME = "fcvt.s.lu";
        };
      };
    };

    
    //-------------------------------------------------------------------------
    // Bit Pattern Move
    //-------------------------------------------------------------------------
    
    struct Fmv {
      struct x {
        struct w : public InstrRs2Op<w, 0, FMT::S, Funct5::FMV_X_W, 0, RegRd, FRegRs1> {
          constexpr static std::string_view NAME = "fmv.x.w";
        };
      };
      struct w {
        struct x : public InstrRs2Op<x, 0, FMT::S, Funct5::FMV_W_X, 0, FRegRd, RegRs1> {
          constexpr static std::string_view NAME = "fmv.w.x";
        };
      };
    };


    //-------------------------------------------------------------------------
    // Float Comparisons
    //-------------------------------------------------------------------------
    
    enum class Comp : funct3_t {
      EQ = 0b010,
      LT = 0b001,
      LE = 0b000
    };

    struct Feq {
      struct s : public InstrOp3<s, Comp::EQ, FMT::S, Funct5::FCMP, RegRd, FRegRs1> {
        constexpr static std::string_view NAME = "feq.s";
      };
    };
    struct Flt {
      struct s : public InstrOp3<s, Comp::LT, FMT::S, Funct5::FCMP, RegRd, FRegRs1> {
        constexpr static std::string_view NAME = "flt.s";
      };
    };
    struct Fle {
      struct s : public InstrOp3<s, Comp::LE, FMT::S, Funct5::FCMP, RegRd, FRegRs1> {
        constexpr static std::string_view NAME = "fle.s";
      };
    };


    //-------------------------------------------------------------------------
    // Floating Classify
    //-------------------------------------------------------------------------
    
    struct Fclass {
      struct s : public InstrRs2Op<s, 1, FMT::S, Funct5::FCLASS, 0, RegRd, FRegRs1> {
        constexpr static std::string_view NAME = "fclass.s";
      };
    };

  } // namespace TypeR

  namespace TypeR4 {
    template <typename InstrImpl, OpcodeID opcode, FMT fmt>
    struct InstrRM : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<opcode>,
                             OpPartFmt<fmt>> {};
      struct Fields : public FieldSet<FRegRd, FRegRs1, FRegRs2, FRegRs3, ImmRM> {};
    };

    struct Fmadd {
      struct s : public InstrRM<s, OpcodeID::MADD, FMT::S> {
        constexpr static std::string_view NAME = "fmadd.s";
      };
    };
    struct Fmsub {
      struct s : public InstrRM<s, OpcodeID::MSUB, FMT::S> {
        constexpr static std::string_view NAME = "fmsub.s";
      };
    };
    struct Fnmsub {
      struct s : public InstrRM<s, OpcodeID::NMSUB, FMT::S> {
        constexpr static std::string_view NAME = "fnmsub.s";
      };
    };
    struct Fnmadd {
      struct s : public InstrRM<s, OpcodeID::NMADD, FMT::S> {
        constexpr static std::string_view NAME = "fnmadd.s";
      };
    };
  } // namespace TypeR4

  namespace TypeCSR {
    template <unsigned tokenIndex>
    struct PseudoReg : public Ripes::PseudoReg<tokenIndex, RV_GPRInfo> {};

    template <typename PseudoInstrImpl>
    struct CsrRead : public PseudoInstruction<PseudoInstrImpl> {
      struct Fields : public FieldSet<PseudoReg> {};

      static Result<std::vector<LineTokens>>
      expander(const PseudoInstruction<PseudoInstrImpl> &,
               const TokenizedSrcLine &line, const SymbolMap &) {
        LineTokensVec v;
        
        // {pseudo} rd  >> expands to: {real} rd, csr, x0
        v.push_back( LineTokens()
            << Token("csrrs")
            << line.tokens.at(1)
            << QString(PseudoInstrImpl::CSR.data())
            << Token("x0") );
        return v;
      }
    };
    
    template <typename PseudoInstrImpl>
    struct CsrReadWrite : public PseudoInstruction<PseudoInstrImpl> {
      struct Fields : public FieldSet<PseudoReg, PseudoReg> {};

      static Result<std::vector<LineTokens>>
      expander(const PseudoInstruction<PseudoInstrImpl> &,
               const TokenizedSrcLine &line, const SymbolMap &) {
        LineTokensVec v;
        
        // {pseudo} rd, rs1  >> expands to: {real} rd, csr, rs1
        v.push_back( LineTokens()
            << Token("csrrw")
            << line.tokens.at(1)
            << QString(PseudoInstrImpl::CSR.data())
            << line.tokens.at(2) );
        return v;
      }
    };

    struct Frcsr : public CsrRead<Frcsr> {
      constexpr static std::string_view NAME = "frcsr";
      constexpr static std::string_view CSR = "fcsr";
    };
    
    struct Fscsr : public CsrReadWrite<Fscsr> {
      constexpr static std::string_view NAME = "fscsr";
      constexpr static std::string_view CSR = "fcsr";
    };
    
    struct Frrm : public CsrRead<Frrm> {
      constexpr static std::string_view NAME = "frrm";
      constexpr static std::string_view CSR = "frm";
    };
    
    struct Fsrm : public CsrReadWrite<Fsrm> {
      constexpr static std::string_view NAME = "fsrm";
      constexpr static std::string_view CSR = "frm";
    };
    
    struct Frflags : public CsrRead<Frflags> {
      constexpr static std::string_view NAME = "frflags";
      constexpr static std::string_view CSR = "fflags";
    };
    
    struct Fsflags : public CsrReadWrite<Fsflags> {
      constexpr static std::string_view NAME = "fsflags";
      constexpr static std::string_view CSR = "fflags";
    };
  }

  namespace TypePseudo {
    template <unsigned tokenIndex>
    struct PseudoReg : public Ripes::PseudoReg<tokenIndex, RV_GPRInfo> {};
    
    template <unsigned tokenIndex>
    struct PseudoRegF : public Ripes::PseudoReg<tokenIndex, RV_FPRInfo> {};


    template <typename PseudoInstrImpl>
    struct PseudoInstrLoadStore : public PseudoInstruction<PseudoInstrImpl> {
      struct Fields : public FieldSet<PseudoRegF, PseudoImm, PseudoReg> {};

      static Result<std::vector<LineTokens>>
      expander(const PseudoInstruction<PseudoInstrImpl> &,
              const TokenizedSrcLine &line, const SymbolMap &) {
        LineTokensVec v;
        
        if (line.tokens.at(2).hasRelocation()) {
          return Result<std::vector<LineTokens>>(
            Error(0, "Pseudo load/store instructions only support relocatable targets."
                     "Default to non-pseudo load/store instruction.")
            );
        }

        bool canConvert;
        (void) getImmediate(line.tokens.at(2), canConvert);
        if (canConvert) {
          return Result<std::vector<LineTokens>>(
            Error(0, "PseudoImm is not symbol."
                     "Default to non-pseudo load/store instruction.")
            );
        }

        v.push_back(LineTokens() << Token("auipc") << line.tokens.at(3)
                                 << Token(line.tokens.at(2), "%pcrel_hi"));
        v.push_back(LineTokens()
                    << QString(PseudoInstrImpl::NAME.data()) << line.tokens.at(1)
                    << Token(QString("(%1 + 4)").arg(line.tokens.at(2)),
                            "%pcrel_lo")
                    << line.tokens.at(3));
        
        return Result<LineTokensVec>(v);
      }
    };

    struct Flw : public PseudoInstrLoadStore<Flw> {
      constexpr static std::string_view NAME = "flw";
    };
    
    struct Fsw : public PseudoInstrLoadStore<Fsw> {
      constexpr static std::string_view NAME = "fsw";
    };
    

    template <typename PseudoInstrImpl>
    struct PseudoFloatInstr : public PseudoInstruction<PseudoInstrImpl> {
      struct Fields : public FieldSet<PseudoRegF, PseudoRegF> {};

      static Result<std::vector<LineTokens>> expander(const PseudoInstruction<PseudoInstrImpl> &,
                                                      const TokenizedSrcLine &line,
                                                      const SymbolMap &) {
        LineTokensVec v;
        // {Fpseudo} rd, rs, rs
        v.push_back( LineTokens()
            << QString(PseudoInstrImpl::PSEUDO_FUNC.data())
            << line.tokens.at(1)
            << line.tokens.at(2)
            << line.tokens.at(2) );
        return v;
      }
    };

    struct Fmv {
      struct s : public PseudoFloatInstr<s> {
        constexpr static std::string_view PSEUDO_FUNC = "fsgnj.s";
        constexpr static std::string_view NAME = "fmv.s";
      };
    };

    struct Fabs {
      struct s : public PseudoFloatInstr<s> {
        constexpr static std::string_view PSEUDO_FUNC = "fsgnjx.s";
        constexpr static std::string_view NAME = "fabs.s";
      };
    };

    struct Fneg {
      struct s : public PseudoFloatInstr<s> {
        constexpr static std::string_view PSEUDO_FUNC = "fsgnjn.s";
        constexpr static std::string_view NAME = "fneg.s";
      };
    };


    template <typename PseudoInstrImpl, template<unsigned> class rd, template<unsigned> class rs1>
    struct PseudoAlias : public PseudoInstruction<PseudoInstrImpl> {
      struct Fields : public FieldSet<rd, rs1> {};

      static Result<std::vector<LineTokens>> expander(const PseudoInstruction<PseudoInstrImpl> &,
                                                      const TokenizedSrcLine &line,
                                                      const SymbolMap &) {
        LineTokensVec v;
        // {Fpseudo} rd, rs
        v.push_back( LineTokens()
            << QString(PseudoInstrImpl::PSEUDO_FUNC.data())
            << line.tokens.at(1)
            << line.tokens.at(2) );
        return v;
      }
    };

    struct FmvAlias {
      struct x {
        struct s : public PseudoAlias<s, PseudoReg, PseudoRegF> {
          constexpr static std::string_view PSEUDO_FUNC = "fmv.x.w";
          constexpr static std::string_view NAME = "fmv.x.s";
        };
      };
      struct s {
        struct x : public PseudoAlias<x, PseudoRegF, PseudoReg> {
          constexpr static std::string_view PSEUDO_FUNC = "fmv.w.x";
          constexpr static std::string_view NAME = "fmv.s.x";
        };
      };
    };
  } // namespace TypePseudo
// clang-format on

}; // namespace ExtF

} // namespace RVISA
} // namespace Ripes
