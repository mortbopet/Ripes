#pragma once

#include <bitset>
#include <iostream>

#include "pseudoinstruction.h"
#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

// clang-format off
namespace ExtF {
  template <typename RegImpl, unsigned tokenIndex, typename Range>
  struct FPR_Reg : public Reg<RegImpl, tokenIndex, Range, RV_FPRInfo> {};

  /// The RISC-V Rs1 field contains a float source register index.
  /// It is defined as a 5-bit field in bits 15-19 of the instruction
  template <unsigned tokenIndex>
  struct FRegRs1
      : public FPR_Reg<FRegRs1<tokenIndex>, tokenIndex, BitRange<15, 19>> {
    constexpr static std::string_view getName() { return "rs1"; }
  };

  /// The RISC-V Rs2 field contains a float source register index.
  /// It is defined as a 5-bit field in bits 20-24 of the instruction
  template <unsigned tokenIndex>
  struct FRegRs2
      : public FPR_Reg<FRegRs2<tokenIndex>, tokenIndex, BitRange<20, 24>> {
    constexpr static std::string_view getName() { return "rs2"; }
  };

  /// The RISC-V Rs2 field contains a float source register index.
  /// It is defined as a 5-bit field in bits 20-24 of the instruction
  template <unsigned tokenIndex>
  struct FRegRs3
      : public FPR_Reg<FRegRs2<tokenIndex>, tokenIndex, BitRange<27, 31>> {
    constexpr static std::string_view getName() { return "rs3"; }
  };

  /// The RISC-V Rd field contains a float destination register index.
  /// It is defined as a 5-bit field in bits 7-11 of the instruction
  template <unsigned tokenIndex>
  struct FRegRd : public FPR_Reg<FRegRd<tokenIndex>, tokenIndex, BitRange<7, 11>> {
    constexpr static std::string_view getName() { return "rd"; }
  };

  enum class Width : unsigned {
    W = 0b010,  // Word
    D = 0b011,  // Double
    H = 0b001,  // Half
    Q = 0b100   // Quad
  };
  template <Width width, unsigned N = 32>
  struct OpPartWidth : public OpPart<static_cast<unsigned>(width), BitRange<12, 14, N>> {};

  typedef unsigned rm_field_t;
  enum class RoundMode : rm_field_t {
    RNE = 0b000, // round to nearest
    RTZ = 0b001, // round towards zero
    RDN = 0b010, // round down (towards -inf)
    RUP = 0b011, // round up   (towards +inf)
    RMM = 0b100, // round to nearest, ties to max magnitude
    _RSVED01 = 0b101, // reserved
    _RSVED10 = 0b110, // reserved
    DYN = 0b111 // dynamic rounding
  };
  template <auto rm, unsigned N = 32>
  struct OpPartRM : public OpPart<static_cast<unsigned>(rm), BitRange<12, 14, N>> {};

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
      struct Fields : public FieldSet<FRegRd, FRegRs1, ImmCommon12> {};
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
      struct Fields : public FieldSet<FRegRs2, ImmS, FRegRs1> {};
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

    static constexpr rm_field_t rm_MIN = 0b000;
    static constexpr rm_field_t rm_MAX = 0b001;

    template <Funct5 funct5, unsigned N = 32>
    struct OpPartFunct5 : public OpPart<static_cast<unsigned>(funct5), BitRange<27, 31, N>> {};

    template <typename InstrImpl, auto rm, FMT fmt, Funct5 funct5>
    struct Instr : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::OP_FP>,
                             OpPartRM<rm>,
                             OpPartFmt<fmt>,
                             OpPartFunct5<funct5>> {};
      struct Fields : public FieldSet<FRegRd, FRegRs1, FRegRs2> {};
    };

    template <typename InstrImpl, auto rm, FMT fmt, Funct5 funct5, auto rs2>
    struct InstrRs2 : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<OpcodeID::OP_FP>,
                             OpPartRM<rm>,
                             OpPartFmt<fmt>,
                             OpPart<static_cast<unsigned>(rs2), BitRange<20, 24>>,
                             OpPartFunct5<funct5>> {};
      struct Fields : public FieldSet<FRegRd, FRegRs1> {};
    };


    //-------------------------------------------------------------------------
    // Float Arithmetic
    //-------------------------------------------------------------------------

    struct Fadd {
      struct s : public Instr<s, RoundMode::DYN, FMT::S, Funct5::FADD> {
        constexpr static std::string_view NAME = "fadd.s";
      };
    };
    struct Fsub {
      struct s : public Instr<s, RoundMode::DYN, FMT::S, Funct5::FSUB> {
        constexpr static std::string_view NAME = "fsub.s";
      };
    };
    struct Fmul {
      struct s : public Instr<s, RoundMode::DYN, FMT::S, Funct5::FMUL> {
        constexpr static std::string_view NAME = "fmul.s";
      };
    };
    struct Fdiv {
      struct s : public Instr<s, RoundMode::DYN, FMT::S, Funct5::FDIV> {
        constexpr static std::string_view NAME = "fdiv.s";
      };
    };

    struct Fsqrt {
      struct s : public InstrRs2<s, RoundMode::DYN, FMT::S, Funct5::FSQRT, 0> {
        constexpr static std::string_view NAME = "fsqrt.s";
      };
    };

    struct Fmin {
      struct s : public Instr<s, rm_MIN, FMT::S, Funct5::FMIN_MAX> {
        constexpr static std::string_view NAME = "fmin.s";
      };
    };
    struct Fmax {
      struct s : public Instr<s, rm_MAX, FMT::S, Funct5::FMIN_MAX> {
        constexpr static std::string_view NAME = "fmax.s";
      };
    };


    //-------------------------------------------------------------------------
    // Float Sign-Injection
    //-------------------------------------------------------------------------
    
    enum class SgnJ : rm_field_t {
      FSGNJ  = 0b000,
      FSGNJN = 0b001,
      FSGNJX = 0b010
    };

    struct Fsgnj {
      struct s : public Instr<s, SgnJ::FSGNJ, FMT::S, Funct5::FSGNJ> {
        constexpr static std::string_view NAME = "fsgnj.s";
      };
    };
    struct Fsgnjn {
      struct s : public Instr<s, SgnJ::FSGNJN, FMT::S, Funct5::FSGNJ> {
        constexpr static std::string_view NAME = "fsgnjn.s";
      };
    };
    struct Fsgnjx {
      struct s : public Instr<s, SgnJ::FSGNJX, FMT::S, Funct5::FSGNJ> {
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
    using Instr_Int_Fmt = InstrRs2<InstrImpl, RoundMode::DYN, fmt, Funct5::FCVT_INT_FMT, cvt>;
    
    template <typename InstrImpl, FMT fmt, Cvt_Int cvt>
    using Instr_Fmt_Int = InstrRs2<InstrImpl, RoundMode::DYN, fmt, Funct5::FCVT_FMT_INT, cvt>;

    struct Fcvt {
      struct w {
        struct s : public Instr_Int_Fmt<s, FMT::S, Cvt_Int::W> {
          constexpr static std::string_view NAME = "fcvt.w.s";
        };
      };
      
      struct wu {
        struct s : public Instr_Int_Fmt<s, FMT::S, Cvt_Int::WU> {
          constexpr static std::string_view NAME = "fcvt.wu.s";
        };
      };
      
      struct l {
        struct s : public Instr_Int_Fmt<s, FMT::S, Cvt_Int::L> {
          constexpr static std::string_view NAME = "fcvt.l.s";
        };
      };
      
      struct lu {
        struct s : public Instr_Int_Fmt<s, FMT::S, Cvt_Int::LU> {
          constexpr static std::string_view NAME = "fcvt.lu.s";
        };
      };

      struct s {
        struct w : public Instr_Fmt_Int<w, FMT::S, Cvt_Int::W> {
          constexpr static std::string_view NAME = "fcvt.s.w";
        };
        struct wu : public Instr_Fmt_Int<wu, FMT::S, Cvt_Int::WU> {
          constexpr static std::string_view NAME = "fcvt.s.wu";
        };
        struct l : public Instr_Fmt_Int<l, FMT::S, Cvt_Int::L> {
          constexpr static std::string_view NAME = "fcvt.s.l";
        };
        struct lu : public Instr_Fmt_Int<lu, FMT::S, Cvt_Int::LU> {
          constexpr static std::string_view NAME = "fcvt.s.lu";
        };
      };
    };

    
    //-------------------------------------------------------------------------
    // Bit Pattern Move
    //-------------------------------------------------------------------------
    
    struct Fmv {
      struct x {
        struct w : public InstrRs2<w, 0, FMT::S, Funct5::FMV_X_W, 0> {
          constexpr static std::string_view NAME = "fmv.x.w";
        };
      };
      struct w {
        struct x : public InstrRs2<x, 0, FMT::S, Funct5::FMV_W_X, 0> {
          constexpr static std::string_view NAME = "fmv.w.x";
        };
      };
    };


    //-------------------------------------------------------------------------
    // Float Comparisons
    //-------------------------------------------------------------------------
    
    enum class Comp : rm_field_t {
      EQ = 0b010,
      LT = 0b001,
      LE = 0b000
    };

    struct Feq {
      struct s : public Instr<s, Comp::EQ, FMT::S, Funct5::FCMP> {
        constexpr static std::string_view NAME = "feq.s";
      };
    };
    struct Flt {
      struct s : public Instr<s, Comp::LT, FMT::S, Funct5::FCMP> {
        constexpr static std::string_view NAME = "flt.s";
      };
    };
    struct Fle {
      struct s : public Instr<s, Comp::LE, FMT::S, Funct5::FCMP> {
        constexpr static std::string_view NAME = "fle.s";
      };
    };


    //-------------------------------------------------------------------------
    // Floating Classify
    //-------------------------------------------------------------------------
    
    struct Fclass {
      struct s : public InstrRs2<s, 1, FMT::S, Funct5::FCLASS, 0> {
        constexpr static std::string_view NAME = "fclass.s";
      };
    };

  } // namespace TypeR

  namespace TypeR4 {
    template <typename InstrImpl, OpcodeID opcode, RoundMode rm, FMT fmt>
    struct Instr : public RV_Instruction<InstrImpl> {
      struct Opcode
          : public OpcodeSet<OpPartOpcode<opcode>,
                             OpPartRM<rm>,
                             OpPartFmt<fmt>> {};
      struct Fields : public FieldSet<FRegRd, FRegRs1, FRegRs2, FRegRs3> {};
    };

    struct Fmadd {
      struct s : public Instr<s, OpcodeID::MADD, RoundMode::DYN, FMT::S> {
        constexpr static std::string_view NAME = "fmadd.s";
      };
    };
    struct Fmsub {
      struct s : public Instr<s, OpcodeID::MSUB, RoundMode::DYN, FMT::S> {
        constexpr static std::string_view NAME = "fmsub.s";
      };
    };
    struct Fnmsub {
      struct s : public Instr<s, OpcodeID::NMSUB, RoundMode::DYN, FMT::S> {
        constexpr static std::string_view NAME = "fnmsub.s";
      };
    };
    struct Fnmadd {
      struct s : public Instr<s, OpcodeID::NMADD, RoundMode::DYN, FMT::S> {
        constexpr static std::string_view NAME = "fnmadd.s";
      };
    };
  } // namespace TypeR4

  namespace TypePseudo {

    template <unsigned tokenIndex>
    struct PseudoReg : public Ripes::PseudoReg<tokenIndex, RV_FPRInfo> {};

    template <typename PseudoInstrImpl>
    struct PseudoInstrLoadStore : public PseudoInstruction<PseudoInstrImpl> {
      struct Fields : public FieldSet<PseudoReg, PseudoImm, PseudoReg> {};

      static Result<std::vector<LineTokens>>
      expander(const PseudoInstruction<PseudoInstrImpl> &,
              const TokenizedSrcLine &line, const SymbolMap &) {
        LineTokensVec v;
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
      struct Fields : public FieldSet<PseudoReg, PseudoReg> {};

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


    template <typename PseudoInstrImpl>
    struct PseudoAlias : public PseudoInstruction<PseudoInstrImpl> {
      struct Fields : public FieldSet<PseudoReg, PseudoReg> {};

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
        struct s : public PseudoAlias<s> {
          constexpr static std::string_view PSEUDO_FUNC = "fmv.x.w";
          constexpr static std::string_view NAME = "fmv.x.s";
        };
      };
      struct s {
        struct x : public PseudoAlias<x> {
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
