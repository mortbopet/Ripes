#pragma once

#include "pseudoinstruction.h"
#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

namespace ExtC {

constexpr unsigned INSTR_BITS = 16;

template <unsigned offset, unsigned start, unsigned stop>
using ImmPart = ImmPartBase<offset, BitRange<start, stop, 16>>;
template <unsigned start, unsigned stop>
using BitRange = Ripes::BitRange<start, stop, 16>;
template <QuadrantID quadrantID>
using OpPartQuadrant = RVISA::OpPartQuadrant<quadrantID, 16>;

template <unsigned funct3>
struct OpPartFunct3 : public OpPart<funct3, BitRange<13, 15>> {};

template <typename InstrImpl>
struct RVC_Instruction : public Instruction<InstrImpl> {
  constexpr static unsigned instrBits() { return INSTR_BITS; }
};

enum class Funct2Offset { OFFSET5 = 5, OFFSET10 = 10 };

/// All RISC-V Funct2 opcode parts are defined as a 2-bit field in bits 5-6 or
/// 10-11 of the instruction
template <unsigned funct2, Funct2Offset offset = Funct2Offset::OFFSET5>
struct OpPartFunct2
    : public OpPart<static_cast<unsigned>(funct2),
                    BitRange<static_cast<unsigned>(offset),
                             static_cast<unsigned>(offset) + 1>> {};

/// All RISC-V Funct4 opcode parts are defined as a 4-bit field in bits 12-15 of
/// the instruction
template <unsigned funct4>
struct OpPartFunct4
    : public OpPart<static_cast<unsigned>(funct4), BitRange<12, 15>> {};

/// The RV-C Rs1 field contains a source register index.
/// It is defined as a 5-bit field in bits 7-11 of the instruction
template <unsigned tokenIndex>
struct RegRs1
    : public GPR_Reg<RegRs1<tokenIndex>, tokenIndex, BitRange<7, 11>> {
  constexpr static std::string_view NAME = "rs1";
};

/// The RV-C Rs2 field contains a source register index.
/// It is defined as a 5-bit field in bits 2-6 of the instruction
template <unsigned tokenIndex>
struct RegRs2 : public GPR_Reg<RegRs2<tokenIndex>, tokenIndex, BitRange<2, 6>> {
  constexpr static std::string_view NAME = "rs2";
};

/// The RV-C Rs1' field contains a source register index.
/// It is defined as a 3-bit field in bits 7-9 of the instruction
template <unsigned tokenIndex>
struct RegRs1Prime
    : public GPR_Reg<RegRs1Prime<tokenIndex>, tokenIndex, BitRange<7, 9>> {
  constexpr static std::string_view NAME = "rs1'";
};

/// The RV-C Rs2' field contains a source register index.
/// It is defined as a 3-bit field in bits 2-4 of the instruction
template <unsigned tokenIndex>
struct RegRs2Prime
    : public GPR_Reg<RegRs2Prime<tokenIndex>, tokenIndex, BitRange<2, 4>> {
  constexpr static std::string_view NAME = "rs2'";
};

/// The RV-C Rd' field contains a destination register
/// index.
/// It is defined as a 3-bit field in bits 2-4 of the instruction
template <unsigned tokenIndex>
struct RegRdPrime
    : public GPR_Reg<RegRdPrime<tokenIndex>, tokenIndex, BitRange<2, 4>> {
  constexpr static std::string_view NAME = "rd'";
};

/// An RV-C immediate field with an input width of 6 bits.
/// Used in the following instructions:
///  - C.ADDI (signed)
///  - C.ADDIW (signed)
///  - C.SLLI (unsigned)
///  - C.LI (signed)
///
/// It is defined as:
///  - Imm[5]   = Inst[12]
///  - Imm[4:0] = Inst[6:2]
template <unsigned tokenIndex, Repr repr>
struct ImmCommon6
    : public Imm<tokenIndex, 6, repr,
                 ImmPartSet<ImmPart<5, 12, 12>, ImmPart<0, 2, 6>>> {
  constexpr static unsigned VALID_INDEX = 1;
  static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
};
template <unsigned tokenIndex>
using ImmCommon6_S = ImmCommon6<tokenIndex, Repr::Signed>;
template <unsigned tokenIndex>
using ImmCommon6_U = ImmCommon6<tokenIndex, Repr::Unsigned>;

/// An RV-C immediate field with an input width of 7 bits.
/// Used in the following instructions:
///  - C.LW
///  - C.FLW
///  - C.SW
///  - C.FSW
///  - C.SD
///  - C.FSD
///
/// It is defined as:
///  - Imm[6]   = Inst[5]
///  - Imm[5:3] = Inst[12:10]
///  - Imm[2]   = Inst[6]
///  - Imm[1:0] = 0
template <unsigned tokenIndex, Repr repr>
struct ImmCommon7
    : public Imm<
          tokenIndex, 7, repr,
          ImmPartSet<ImmPart<6, 5, 5>, ImmPart<3, 10, 12>, ImmPart<2, 6, 6>>> {
  constexpr static unsigned VALID_INDEX = 2;
  static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
};
template <unsigned tokenIndex>
using ImmCommon7_S = ImmCommon7<tokenIndex, Repr::Signed>;
template <unsigned tokenIndex>
using ImmCommon7_U = ImmCommon7<tokenIndex, Repr::Unsigned>;

namespace TypeCA {

enum class Funct2 { SUB = 0b00, XOR_ADD = 0b01, OR = 0b10, AND = 0b11 };
enum class Funct6 { DEFAULT = 0b100011, WIDE = 0b100111 };

/// All RV-C Funct6 opcode parts are defined as a 6-bit field in bits 10-15
/// of the Funct6
template <Funct6 funct6>
struct OpPartFunct6
    : public OpPart<static_cast<unsigned>(funct6), BitRange<10, 15>> {};

/// The RV-C Rd'/Rs1' field contains a source or destination
/// register index.
/// It is defined as a 3-bit field in bits 7-9 of the instruction
template <unsigned tokenIndex>
struct RegRdRs1Prime
    : public GPR_Reg<RegRdRs1Prime<tokenIndex>, tokenIndex, BitRange<7, 9>> {
  constexpr static std::string_view NAME = "rd'/rs1'";
};

template <typename InstrImpl, Funct2 funct2, Funct6 funct6 = Funct6::DEFAULT>
struct Instr : public RVC_Instruction<InstrImpl> {
  struct Opcode : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT1>,
                                   OpPartFunct2<static_cast<unsigned>(funct2)>,
                                   OpPartFunct6<funct6>> {};
  struct Fields : public FieldSet<RegRdRs1Prime, RegRs2Prime> {};
};

struct CSub : Instr<CSub, Funct2::SUB> {
  constexpr static std::string_view NAME = "c.sub";
};

struct CXor : Instr<CXor, Funct2::XOR_ADD> {
  constexpr static std::string_view NAME = "c.xor";
};

struct COr : Instr<COr, Funct2::OR> {
  constexpr static std::string_view NAME = "c.or";
};

struct CAnd : Instr<CAnd, Funct2::AND> {
  constexpr static std::string_view NAME = "c.and";
};

struct CSubw : Instr<CSubw, Funct2::SUB, Funct6::WIDE> {
  constexpr static std::string_view NAME = "c.subw";
};

struct CAddw : Instr<CAddw, Funct2::XOR_ADD, Funct6::WIDE> {
  constexpr static std::string_view NAME = "c.addw";
};

} // namespace TypeCA

namespace TypeCI {

enum class Funct3 {
  LWSP = 0b010,
  FLWSP = 0b011,
  LDSP = 0b011,
  ADDIW = 0b001,
  FLDSP = 0b001,
  SLLI = 0b000,
  LI = 0b010,
  LUI = 0b011,
  ADDI = 0b000,
  ADDI16SP = 0b011
};

/// The RV-C Rd/Rs1 field contains a source or destination register
/// index.
/// It is defined as a 5-bit field in bits 7-11 of the instruction
template <unsigned tokenIndex>
struct RegRdRs1
    : public GPR_Reg<RegRdRs1<tokenIndex>, tokenIndex, BitRange<7, 11>> {
  constexpr static std::string_view NAME = "rd/rs1'";
};

constexpr static unsigned VALID_INDEX = 1;

template <typename InstrImpl, QuadrantID quadrantID, Funct3 funct3,
          template <unsigned> typename ImmType>
struct Instr : public RVC_Instruction<InstrImpl> {
  struct Opcode
      : public OpcodeSet<OpPartQuadrant<quadrantID>,
                         OpPartFunct3<static_cast<unsigned>(funct3)>> {};
  struct Fields : public FieldSet<RegRdRs1, ImmType> {};
};

/// An RV-C unsigned immediate field with an input width of 8 bits.
/// Used in C.LSWP and C.FLWSP instructions.
///
/// It is defined as:
///  - Imm[7:6] = Inst[3:2]
///  - Imm[5]   = Inst[12]
///  - Imm[4:2] = Inst[6:4]
///  - Imm[1:0] = 0
template <unsigned tokenIndex>
struct ImmLwsp
    : public Imm<
          tokenIndex, 8, Repr::Unsigned,
          ImmPartSet<ImmPart<6, 2, 3>, ImmPart<5, 12, 12>, ImmPart<2, 4, 6>>> {
  static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
};

struct CLwsp
    : public Instr<CLwsp, QuadrantID::QUADRANT2, Funct3::LWSP, ImmLwsp> {
  constexpr static std::string_view NAME = "c.lwsp";
};

struct CFlwsp
    : public Instr<CFlwsp, QuadrantID::QUADRANT2, Funct3::FLWSP, ImmLwsp> {
  constexpr static std::string_view NAME = "c.flwsp";
};

/// An RV-C unsigned immediate field with an input width of 9 bits.
/// Used in C.LDWP and C.FLDSP instructions.
///
/// It is defined as:
///  - Imm[8:6] = Inst[4:2]
///  - Imm[5]   = Inst[12]
///  - Imm[4:3] = Inst[6:5]
///  - Imm[2:0] = 0
template <unsigned tokenIndex>
struct ImmLdsp
    : public Imm<
          tokenIndex, 9, Repr::Unsigned,
          ImmPartSet<ImmPart<6, 2, 4>, ImmPart<5, 12, 12>, ImmPart<3, 5, 6>>> {
  static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
};

struct CLdsp
    : public Instr<CLdsp, QuadrantID::QUADRANT2, Funct3::LDSP, ImmLdsp> {
  constexpr static std::string_view NAME = "c.ldsp";
};

struct CAddiw
    : public Instr<CAddiw, QuadrantID::QUADRANT1, Funct3::ADDIW, ImmCommon6_S> {
  constexpr static std::string_view NAME = "c.addiw";
};

struct CFldsp
    : public Instr<CFldsp, QuadrantID::QUADRANT2, Funct3::FLDSP, ImmLdsp> {
  constexpr static std::string_view NAME = "c.fldsp";
};

struct CSlli
    : public Instr<CSlli, QuadrantID::QUADRANT2, Funct3::SLLI, ImmCommon6_U> {
  constexpr static std::string_view NAME = "c.slli";
};

struct CLi
    : public Instr<CLi, QuadrantID::QUADRANT1, Funct3::LI, ImmCommon6_S> {
  constexpr static std::string_view NAME = "c.li";
};

/// An RV-C signed immediate field with an input width of 18 bits.
/// Used in C.LUI instructions.
///
/// It is defined as:
///  - Imm[17]    = Inst[12]
///  - Imm[16:12] = Inst[6:2]
///  - Imm[12:0]  = 0
template <unsigned tokenIndex>
struct ImmLui : public Imm<tokenIndex, 18, Repr::Signed,
                           ImmPartSet<ImmPart<17, 12, 12>, ImmPart<12, 2, 6>>> {
  static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
};

struct CLui : public Instr<CLui, QuadrantID::QUADRANT1, Funct3::LUI, ImmLui> {
  constexpr static std::string_view NAME = "c.lui";
  CLui() {
    addExtraMatchCond([](Instr_T instr) {
      unsigned rd = (instr >> 7) & 0b11111;
      return rd != 0 && rd != 2;
    });
  }
};

struct CAddi16Sp : public RVC_Instruction<CAddi16Sp> {
  template <unsigned tokenIndex>
  struct Imm
      : public Ripes::Imm<
            tokenIndex, 10, Repr::Signed,
            ImmPartSet<ImmPart<9, 12, 12>, ImmPart<7, 3, 4>, ImmPart<5, 2, 2>,
                       ImmPart<4, 6, 6>, ImmPart<6, 5, 5>>> {
    constexpr static unsigned VALID_INDEX = 0;
    static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
  };
  struct ConstantOpPart : public OpPart<2, BitRange<7, 11>> {};

  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT1>,
                         OpPartFunct3<static_cast<unsigned>(Funct3::ADDI16SP)>,
                         ConstantOpPart> {};
  struct Fields : public FieldSet<Imm> {};
  constexpr static std::string_view NAME = "c.addi16sp";
  CAddi16Sp() {
    addExtraMatchCond([](Instr_T instr) {
      unsigned rd = (instr >> 7) & 0b11111;
      return rd == 2;
    });
  }
};

struct CAddi
    : public Instr<CAddi, QuadrantID::QUADRANT1, Funct3::ADDI, ImmCommon6_S> {
  constexpr static std::string_view NAME = "c.addi";
};

struct CNop : public RVC_Instruction<CNop> {
  /// The 14-bit field from bits 2-15 are set to 0 in a compressed NOP
  /// instruction
  struct OpPart : OpPartZeroes<2, 15, INSTR_BITS> {};

  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT1>, OpPart> {};
  struct Fields : public FieldSet<> {};

  constexpr static std::string_view NAME = "c.nop";
};

} // namespace TypeCI

namespace TypeCSS {

enum class Funct3 { SWSP = 0b110, FSWSP = 0b111, SDSP = 0b111, FSDSP = 0b101 };

constexpr static unsigned VALID_INDEX = 1;

template <typename InstrImpl, Funct3 funct3,
          template <unsigned> typename ImmType>
struct Instr : public RVC_Instruction<InstrImpl> {
  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT2>,
                         OpPartFunct3<static_cast<unsigned>(funct3)>> {};
  struct Fields : public FieldSet<RegRs2, ImmType> {};
};

/// An RV-C unsigned immediate field with an input width of 8 bits.
/// Used in C.SWSP and C.FSWSP instructions.
///
/// It is defined as:
///  - Imm[7:6] = Inst[8:7]
///  - Imm[5:2] = Inst[12:9]
///  - Imm[1:0] = 0
template <unsigned tokenIndex>
struct ImmSwsp : public Imm<tokenIndex, 8, Repr::Unsigned,
                            ImmPartSet<ImmPart<6, 7, 8>, ImmPart<2, 9, 12>>> {
  static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
};

struct CSwsp : public Instr<CSwsp, Funct3::SWSP, ImmSwsp> {
  constexpr static std::string_view NAME = "c.swsp";
};

struct CFswsp : public Instr<CFswsp, Funct3::FSWSP, ImmSwsp> {
  constexpr static std::string_view NAME = "c.fswsp";
};

/// An RV-C unsigned immediate field with an input width of 9 bits.
/// Used in C.SDSP and C.FSDSP instructions.
///
/// It is defined as:
///  - Imm[8:6] = Inst[9:7]
///  - Imm[5:3] = Inst[12:10]
///  - Imm[2:0] = 0
template <unsigned tokenIndex>
struct ImmSdsp : public Imm<tokenIndex, 9, Repr::Unsigned,
                            ImmPartSet<ImmPart<6, 7, 9>, ImmPart<3, 10, 12>>> {
  static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
};

struct CSdsp : public Instr<CSdsp, Funct3::SDSP, ImmSdsp> {
  constexpr static std::string_view NAME = "c.sdsp";
};

struct CFsdsp : public Instr<CFsdsp, Funct3::FSDSP, ImmSdsp> {
  constexpr static std::string_view NAME = "c.fsdsp";
};

} // namespace TypeCSS

namespace TypeCL {

enum class Funct3 { LW = 0b010, FLW = 0b011, LD = 0b011, FLD = 0b001 };

constexpr static unsigned VALID_INDEX = 2;

template <typename InstrImpl, Funct3 funct3,
          template <unsigned> typename ImmType>
struct Instr : public RVC_Instruction<InstrImpl> {
  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT0>,
                         OpPartFunct3<static_cast<unsigned>(funct3)>> {};
  struct Fields : public FieldSet<RegRdPrime, RegRs1Prime, ImmType> {};
};

/// An RV-C signed immediate field with an input width of 8 bits.
/// Used in C.LD and C.FLD instructions.
///
/// It is defined as:
///  - Imm[7:6] = Inst[6:5]
///  - Imm[5:3] = Inst[12:10]
///  - Imm[2:0] = 0
template <unsigned tokenIndex>
struct ImmLd : public Imm<tokenIndex, 8, Repr::Signed,
                          ImmPartSet<ImmPart<6, 5, 6>, ImmPart<3, 10, 12>>> {
  static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
};

struct CLw : public Instr<CLw, Funct3::LW, ImmCommon7_S> {
  constexpr static std::string_view NAME = "c.lw";
};

struct CFlw : public Instr<CFlw, Funct3::FLW, ImmCommon7_S> {
  constexpr static std::string_view NAME = "c.flw";
};

struct CLd : public Instr<CLd, Funct3::LD, ImmLd> {
  constexpr static std::string_view NAME = "c.ld";
};

struct CFld : public Instr<CFld, Funct3::FLD, ImmLd> {
  constexpr static std::string_view NAME = "c.fld";
};

} // namespace TypeCL

namespace TypeCS {

enum class Funct3 { SW = 0b110, FSW = 0b111, SD = 0b111, FSD = 0b101 };

template <typename InstrImpl, Funct3 funct3>
struct Instr : public RVC_Instruction<InstrImpl> {
  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT0>,
                         OpPartFunct3<static_cast<unsigned>(funct3)>> {};
  struct Fields : public FieldSet<RegRs2Prime, RegRs1Prime, ImmCommon7_U> {};
};

struct CSw : public Instr<CSw, Funct3::SW> {
  constexpr static std::string_view NAME = "c.sw";
};

struct CFsw : public Instr<CFsw, Funct3::FSW> {
  constexpr static std::string_view NAME = "c.fsw";
};

struct CSd : public Instr<CSd, Funct3::SD> {
  constexpr static std::string_view NAME = "c.sd";
};

struct CFsd : public Instr<CFsd, Funct3::FSD> {
  constexpr static std::string_view NAME = "c.fsd";
};

} // namespace TypeCS

namespace TypeCJ {

enum class Funct3 { J = 0b101, JAL = 0b001 };

constexpr static unsigned VALID_INDEX = 0;

template <typename InstrImpl, Funct3 funct3>
struct Instr : public RVC_Instruction<InstrImpl> {

  /// An RV-C signed immediate field with an input width of 12 bits.
  /// Used in C.J and C.JAL instructions.
  ///
  /// It is defined as:
  ///  - Imm[11]  = Inst[12]
  ///  - Imm[10]  = Inst[8]
  ///  - Imm[9:8] = Inst[10:9]
  ///  - Imm[7]   = Inst[6]
  ///  - Imm[6]   = Inst[7]
  ///  - Imm[5]   = Inst[2]
  ///  - Imm[4]   = Inst[11]
  ///  - Imm[3:1] = Inst[5:3]
  ///  - Imm[0]   = 0
  template <unsigned tokenIndex>
  struct Imm
      : public Ripes::Imm<tokenIndex, 12, Repr::Signed,
                          ImmPartSet<ImmPart<11, 12, 12>, ImmPart<10, 8, 8>,
                                     ImmPart<8, 9, 10>, ImmPart<7, 6, 6>,
                                     ImmPart<6, 7, 7>, ImmPart<5, 2, 2>,
                                     ImmPart<4, 11, 11>, ImmPart<1, 3, 5>>> {
    static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
  };

  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT1>,
                         OpPartFunct3<static_cast<unsigned>(funct3)>> {};
  struct Fields : public FieldSet<Imm> {};
};

struct CJ : public Instr<CJ, Funct3::J> {
  constexpr static std::string_view NAME = "c.j";
};

struct CJal : public Instr<CJal, Funct3::JAL> {
  constexpr static std::string_view NAME = "c.jal";
};

} // namespace TypeCJ

namespace TypeCB {

enum class Funct3 { BEQZ = 0b110, BNEZ = 0b111 };

constexpr static unsigned VALID_INDEX = 1;

template <typename InstrImpl, Funct3 funct3>
struct Instr : public RVC_Instruction<InstrImpl> {
  /// An RV-C signed immediate field with an input width of 9 bits.
  /// Used in C.BEQZ and C.BNEZ instructions.
  ///
  /// It is defined as:
  ///  - Imm[8]   = Inst[12]
  ///  - Imm[7:6] = Inst[6:5]
  ///  - Imm[5]   = Inst[2]
  ///  - Imm[4:3] = Inst[11:10]
  ///  - Imm[2:1] = Inst[4:3]
  ///  - Imm[0]   = 0
  template <unsigned tokenIndex>
  struct Imm
      : public Ripes::Imm<
            tokenIndex, 9, Repr::Signed,
            ImmPartSet<ImmPart<8, 12, 12>, ImmPart<6, 5, 6>, ImmPart<5, 2, 2>,
                       ImmPart<3, 10, 11>, ImmPart<1, 3, 4>>> {
    static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
  };

  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT1>,
                         OpPartFunct3<static_cast<unsigned>(funct3)>> {};
  struct Fields : public FieldSet<RegRs1Prime, Imm> {};
};

struct CBeqz : public Instr<CBeqz, Funct3::BEQZ> {
  constexpr static std::string_view NAME = "c.beqz";
};

struct CBnez : public Instr<CBnez, Funct3::BNEZ> {
  constexpr static std::string_view NAME = "c.bnez";
};

} // namespace TypeCB

namespace TypeCB2 {

enum class Funct2 { SRLI = 0b00, SRAI = 0b01, ANDI = 0b10 };

constexpr static unsigned VALID_INDEX = 1;

template <typename InstrImpl, Funct2 funct2, Repr repr>
struct Instr : public RVC_Instruction<InstrImpl> {
  /// An RV-C immediate field with an input width of 6 bits.
  /// Used in the following instructions:
  ///  - C.SRLI (unsigned)
  ///  - C.SRAI (unsigned)
  ///  - C.ANDI (signed)
  ///
  /// It is defined as:
  ///  - Imm[5]   = Inst[12]
  ///  - Imm[4:0] = Inst[6:2]
  template <unsigned tokenIndex>
  struct Imm
      : public Ripes::Imm<tokenIndex, 6, repr,
                          ImmPartSet<ImmPart<5, 12, 12>, ImmPart<0, 2, 6>>> {
    static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
  };

  struct Opcode
      : public OpcodeSet<
            OpPartQuadrant<QuadrantID::QUADRANT1>,
            OpPartFunct2<static_cast<unsigned>(funct2), Funct2Offset::OFFSET10>,
            OpPartFunct3<0b100>> {};
  struct Fields : public FieldSet<RegRs1Prime, Imm> {};
};

struct CSrli : public Instr<CSrli, Funct2::SRLI, Repr::Unsigned> {
  constexpr static std::string_view NAME = "c.srli";
};

struct CSrai : public Instr<CSrai, Funct2::SRAI, Repr::Unsigned> {
  constexpr static std::string_view NAME = "c.srai";
};

struct CAndi : public Instr<CAndi, Funct2::ANDI, Repr::Signed> {
  constexpr static std::string_view NAME = "c.andi";
};

} // namespace TypeCB2

namespace TypeCIW {

constexpr static unsigned VALID_INDEX = 1;

struct CAddi4spn : public RVC_Instruction<CAddi4spn> {
  /// An RV-C unsigned immediate field with an input width of 10 bits.
  /// Used in the C.ADDI4SPN instruction.
  ///
  /// It is defined as:
  ///  - Imm[9:6] = Inst[10:7]
  ///  - Imm[5:4] = Inst[12:11]
  ///  - Imm[3]   = Inst[5]
  ///  - Imm[2]   = Inst[6]
  ///  - Imm[1:0] = 0
  template <unsigned tokenIndex>
  struct Imm
      : public Ripes::Imm<tokenIndex, 10, Repr::Unsigned,
                          ImmPartSet<ImmPart<6, 7, 10>, ImmPart<4, 11, 12>,
                                     ImmPart<3, 5, 5>, ImmPart<2, 6, 6>>> {
    static_assert(tokenIndex == VALID_INDEX, "Invalid token index");
  };

  struct Opcode : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT0>,
                                   OpPartFunct3<0b000>> {};
  struct Fields : public FieldSet<RegRdPrime, Imm> {};

  constexpr static std::string_view NAME = "c.addi4spn";
};

} // namespace TypeCIW

namespace TypeCR {

enum class Funct4 { MV = 0b1000, ADD = 0b1001 };

template <typename InstrImpl, Funct4 funct4>
struct Instr : public RVC_Instruction<InstrImpl> {
  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT2>,
                         OpPartFunct4<static_cast<unsigned>(funct4)>> {};
  struct Fields : public FieldSet<RegRs1, RegRs2> {};
};

// FIXME disassemble erro with c.jr ?
struct CMv : public Instr<CMv, Funct4::MV> {
  constexpr static std::string_view NAME = "c.mv";
};

struct CAdd : public Instr<CAdd, Funct4::ADD> {
  constexpr static std::string_view NAME = "c.add";
};

} // namespace TypeCR

namespace TypeCR2 {

enum class Funct4 { JR = 0b1000, JALR = 0b1001 };

template <typename InstrImpl, Funct4 funct4>
struct Instr : public RVC_Instruction<InstrImpl> {
  struct Opcode
      : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT2>,
                         OpPartZeroes<2, 6, INSTR_BITS>,
                         OpPartFunct4<static_cast<unsigned>(funct4)>> {};
  struct Fields : public FieldSet<RegRs1> {};
};

struct CJr : public Instr<CJr, Funct4::JR> {
  constexpr static std::string_view NAME = "c.jr";
};

struct CJalr : public Instr<CJalr, Funct4::JALR> {
  CJalr() {
    addExtraMatchCond([](Instr_T instr) {
      unsigned rd = (instr >> 7) & 0b11111;
      return rd != 0;
    });
  }

  AssembleRes assemble(const TokenizedSrcLine &line) override {
    if (line.tokens.size() >= 2 && line.tokens.at(1) == "x0") {
      return Error(line, "c.jalr cannot use register x0");
    }
    return Instr::assemble(line);
  }

  constexpr static std::string_view NAME = "c.jalr";
};

} // namespace TypeCR2

struct CEbreak : public RVC_Instruction<CEbreak> {
  constexpr static unsigned Funct4 = 0b1001;

  struct Opcode : public OpcodeSet<OpPartQuadrant<QuadrantID::QUADRANT2>,
                                   OpPartZeroes<2, 11, INSTR_BITS>,
                                   OpPartFunct4<Funct4>> {};
  struct Fields : public FieldSet<> {};

  constexpr static std::string_view NAME = "c.ebreak";
};

} // namespace ExtC

} // namespace RVISA
} // namespace Ripes
