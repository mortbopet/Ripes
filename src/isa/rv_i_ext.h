#pragma once

#include <bitset>
#include <iostream>

#include "pseudoinstruction.h"
#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

namespace ExtI {

enum class Options {
  shifts64BitVariant, // appends 'w' to 32-bit shift operations, for use in
                      // the 64-bit RISC-V ISA
  LI64BitVariant      // Modifies LI to be able to emit 64-bit constants
};

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions,
               const std::set<Options> &options = {});

/// A RISC-V signed immediate field with a width of 12 bits.
/// Used in L-Type and I-Type instructions.
///
/// It is defined as:
///  - Imm[31:11] = Inst[31]
///  - Imm[10:0]  = Inst[30:20]
template <unsigned tokenIndex>
struct ImmCommon12
    : public Imm<tokenIndex, 12, Repr::Signed, ImmPart<0, 20, 31>> {};

namespace TypeI {

enum class Funct3 : unsigned {
  ADDI = 0b000,
  SLTI = 0b010,
  SLTIU = 0b011,
  XORI = 0b100,
  ORI = 0b110,
  ANDI = 0b111,
};

/// An I-Type RISC-V instruction
template <typename InstrImpl, OpcodeID opcodeID, Funct3 funct3>
struct Instr : public RV_Instruction<InstrImpl> {
  struct ITypeOpcode {
    using RVOpcode = OpPartOpcode<static_cast<unsigned>(opcodeID)>;
    using RVFunct3 = OpPartFunct3<static_cast<unsigned>(funct3)>;
    using Impl = OpcodeImpl<RVOpcode, RVFunct3>;
  };
  struct ITypeFields {
    using Rd = RegRd<0>;
    using Rs1 = RegRs1<1>;
    using Imm = ImmCommon12<2>;
    using Impl = FieldsImpl<Rd, Rs1, Imm>;
  };

  using Opcode = ITypeOpcode;
  using Fields = ITypeFields;
};

template <typename InstrImpl, Funct3 funct3>
using Instr32 = Instr<InstrImpl, OpcodeID::OPIMM, funct3>;
template <typename InstrImpl, Funct3 funct3>
using Instr64 = Instr<InstrImpl, OpcodeID::OPIMM32, funct3>;

struct Addi : public Instr32<Addi, Funct3::ADDI> {
  constexpr static std::string_view Name = "addi";
};

struct Andi : public Instr32<Andi, Funct3::ANDI> {
  constexpr static std::string_view Name = "andi";
};

struct Slti : public Instr32<Slti, Funct3::SLTI> {
  constexpr static std::string_view Name = "slti";
};

struct Sltiu : public Instr32<Sltiu, Funct3::SLTIU> {
  constexpr static std::string_view Name = "sltiu";
};

struct Xori : public Instr32<Xori, Funct3::XORI> {
  constexpr static std::string_view Name = "xori";
};

struct Ori : public Instr32<Ori, Funct3::ORI> {
  constexpr static std::string_view Name = "ori";
};

struct Jalr : public RV_Instruction<Jalr> {
  struct JalrTypeOpcode {
    using RVOpcode = OpPartOpcode<static_cast<unsigned>(RVISA::OpcodeID::JALR)>;
    using RVFunct3 = OpPartFunct3<static_cast<unsigned>(0b000)>;
    using Impl = OpcodeImpl<RVOpcode, RVFunct3>;
  };
  struct JalrTypeFields {
    using Rd = RegRd<0>;
    using Rs1 = RegRs1<1>;
    using Imm = ImmCommon12<2>;
    using Impl = FieldsImpl<Rd, Rs1, Imm>;
  };

  using Opcode = JalrTypeOpcode;
  using Fields = JalrTypeFields;

  constexpr static std::string_view Name = "jalr";
};

} // namespace TypeI

namespace TypeIShift {

enum class Funct3 : unsigned { SLLI = 0b001, SRLI = 0b101, SRAI = 0b101 };
enum class Funct7 : unsigned {
  LEFT_SHIFT = 0b0000000,
  RIGHT_SHIFT = 0b0100000
};

/// A RISC-V unsigned immediate field with a width of 5 bits.
/// Used in IShift32-Type instructions.
///
/// It is defined as:
///  - Imm[4:0] = Inst[24:20]
constexpr static unsigned ImmTokenIndex = 2;
struct ImmIShift32
    : public Imm<ImmTokenIndex, 5, Repr::Unsigned, ImmPart<0, 20, 24>> {};

/// An IShift-Type RISC-V instruction
template <typename InstrImpl, OpcodeID opcodeID, Funct3 funct3, Funct7 funct7>
struct Instr : public RV_Instruction<InstrImpl> {
  struct IShiftTypeOpcode {
    using RVOpcode = OpPartOpcode<static_cast<unsigned>(opcodeID)>;
    using RVFunct3 = OpPartFunct3<static_cast<unsigned>(funct3)>;
    using RVFunct7 = OpPartFunct7<static_cast<unsigned>(funct7)>;
    using Impl = OpcodeImpl<RVOpcode, RVFunct3, RVFunct7>;
  };
  struct IShiftTypeFields {
    using Rd = RegRd<0>;
    using Rs1 = RegRs1<1>;
    using Imm = ImmIShift32;
    using Impl = FieldsImpl<Rd, Rs1, Imm>;
  };

  using Opcode = IShiftTypeOpcode;
  using Fields = IShiftTypeFields;
};

template <typename InstrImpl, Funct3 funct3, Funct7 funct7 = Funct7::LEFT_SHIFT>
using Instr32 = Instr<InstrImpl, OpcodeID::OPIMM, funct3, funct7>;
template <typename InstrImpl, Funct3 funct3, Funct7 funct7 = Funct7::LEFT_SHIFT>
using Instr64 = Instr<InstrImpl, OpcodeID::OPIMM32, funct3, funct7>;

struct Slli : public Instr32<Slli, Funct3::SLLI> {
  constexpr static std::string_view Name = "slli";
};

struct Srli : public Instr32<Srli, Funct3::SRLI> {
  constexpr static std::string_view Name = "srli";
};

struct Srai : public Instr32<Srai, Funct3::SRAI, Funct7::RIGHT_SHIFT> {
  constexpr static std::string_view Name = "srai";
};

struct Slliw : public Instr64<Slliw, Funct3::SLLI> {
  constexpr static std::string_view Name = "slliw";
};

struct Srliw : public Instr64<Srliw, Funct3::SRLI> {
  constexpr static std::string_view Name = "srliw";
};

struct Sraiw : public Instr64<Sraiw, Funct3::SRAI, Funct7::RIGHT_SHIFT> {
  constexpr static std::string_view Name = "sraiw";
};

} // namespace TypeIShift

namespace TypeIShift64 {

enum class Funct3 { SLLI = 0b001, SRLI = 0b101, SRAI = 0b101 };
enum class Funct6 { LEFT_SHIFT = 0b0000000, RIGHT_SHIFT = 0b0100000 };

/// A RISC-V unsigned immediate field with an input width of 6 bits.
/// Used in IShift64-Type instructions.
///
/// It is defined as:
///  - Imm[5:0] = Inst[25:20]
constexpr static unsigned ImmTokenIndex = 2;
struct ImmIShift64
    : public Imm<ImmTokenIndex, 6, Repr::Unsigned, ImmPart<0, 20, 25>> {};

template <typename InstrImpl, OpcodeID opcodeID, Funct3 funct3,
          Funct6 funct6 = Funct6::LEFT_SHIFT>
struct Instr : public RV_Instruction<InstrImpl> {
  struct IShiftTypeOpcode {
    using RVOpcode = OpPartOpcode<static_cast<unsigned>(opcodeID)>;
    using RVFunct3 = OpPartFunct3<static_cast<unsigned>(funct3)>;
    using RVFunct6 = OpPartFunct6<static_cast<unsigned>(funct6)>;
    using Impl = OpcodeImpl<RVOpcode, RVFunct3, RVFunct6>;
  };
  struct IShiftTypeFields {
    using Rd = RegRd<0>;
    using Rs1 = RegRs1<1>;
    using Imm = ImmIShift64;
    using Impl = FieldsImpl<Rd, Rs1, Imm>;
  };

  using Opcode = IShiftTypeOpcode;
  using Fields = IShiftTypeFields;
};

template <typename InstrImpl, Funct3 funct3, Funct6 funct6 = Funct6::LEFT_SHIFT>
using Instr32 = Instr<InstrImpl, OpcodeID::OPIMM, funct3, funct6>;
template <typename InstrImpl, Funct3 funct3, Funct6 funct6 = Funct6::LEFT_SHIFT>
using Instr64 = Instr<InstrImpl, OpcodeID::OPIMM32, funct3, funct6>;

struct Slli : public Instr32<Slli, Funct3::SLLI> {
  constexpr static std::string_view Name = "slli";
};

struct Srli : public Instr32<Srli, Funct3::SRLI> {
  constexpr static std::string_view Name = "srli";
};

struct Srai : public Instr32<Srai, Funct3::SRAI, Funct6::RIGHT_SHIFT> {
  constexpr static std::string_view Name = "srai";
};

} // namespace TypeIShift64

namespace TypeL {

enum class Funct3 : unsigned {
  LB = 0b000,
  LH = 0b001,
  LW = 0b010,
  LBU = 0b100,
  LHU = 0b101,
  LWU = 0b110,
  LD = 0b011
};

/// An L-Type RISC-V instruction
template <typename InstrImpl, Funct3 funct3>
struct Instr : public RV_Instruction<InstrImpl> {
  struct LTypeOpcode {
    using RVOpcode = OpPartOpcode<static_cast<unsigned>(RVISA::OpcodeID::LOAD)>;
    using RVFunct3 = OpPartFunct3<static_cast<unsigned>(funct3)>;
    using Impl = OpcodeImpl<RVOpcode, RVFunct3>;
  };
  struct LTypeFields {
    using Rd = RegRd<0>;
    using Rs1 = RegRs1<1>;
    using Imm = ImmCommon12<2>;
    using Impl = FieldsImpl<Rd, Rs1, Imm>;
  };

  using Opcode = LTypeOpcode;
  using Fields = LTypeFields;
};

struct Lb : public Instr<Lb, Funct3::LB> {
  constexpr static std::string_view Name = "lb";
};

struct Lh : public Instr<Lh, Funct3::LH> {
  constexpr static std::string_view Name = "lh";
};

struct Lw : public Instr<Lw, Funct3::LW> {
  constexpr static std::string_view Name = "lw";
};

struct Lbu : public Instr<Lbu, Funct3::LBU> {
  constexpr static std::string_view Name = "lbu";
};

struct Lhu : public Instr<Lhu, Funct3::LHU> {
  constexpr static std::string_view Name = "lhu";
};

struct Lwu : public Instr<Lwu, Funct3::LWU> {
  constexpr static std::string_view Name = "lwu";
};

struct Ld : public Instr<Ld, Funct3::LD> {
  constexpr static std::string_view Name = "ld";
};

} // namespace TypeL

namespace TypeSystem {

enum class Funct12 { ECALL = 0x000, EBREAK = 0b001 };

/// All RISC-V Funct12 opcode parts are defined as a 12-bit field in bits 20-31
/// of the instruction
template <unsigned funct12>
struct OpPartFunct12 : public OpPart<funct12, BitRange<20, 31>> {};

/// A System-Type RISC-V instruction (ECALL and EBREAK)
template <typename InstrImpl, Funct12 funct12>
struct Instr : public RV_Instruction<InstrImpl> {
  struct SystemTypeOpcode {
    using RVOpcode =
        OpPartOpcode<static_cast<unsigned>(RVISA::OpcodeID::SYSTEM)>;
    using Rd = OpPartZeroes<7, 11>;
    using RVFunct3 = OpPartFunct3<0>;
    using Rs1 = OpPartZeroes<15, 19>;
    using RVFunct12 = OpPartFunct12<static_cast<unsigned>(funct12)>;
    using Impl = OpcodeImpl<RVOpcode, Rd, RVFunct3, Rs1, RVFunct12>;
  };
  struct SystemTypeFields {
    using Impl = FieldsImpl<>;
  };

  using Opcode = SystemTypeOpcode;
  using Fields = SystemTypeFields;
};

struct Ecall : public Instr<Ecall, Funct12::ECALL> {
  constexpr static std::string_view Name = "ecall";
};

} // namespace TypeSystem

namespace TypeU {

/// A RISC-V immediate field with an input width of 32 bits.
/// Used in U-Type instructions.
///
/// It is defined as:
///  - Imm[31:12] = Inst[31:12]
///  - Imm[11:0]  = 0
template <SymbolType symbolType>
struct ImmU : public ImmSym<1, 32, Repr::Hex, ImmPart<0, 12, 31>, symbolType> {
};

/// A U-Type RISC-V instruction
template <typename InstrImpl, RVISA::OpcodeID opcode,
          SymbolType symbolType = SymbolType::None>
struct Instr : public RV_Instruction<InstrImpl> {
  struct UTypeOpcode {
    using RVOpcode = OpPartOpcode<static_cast<unsigned>(opcode)>;
    using Impl = OpcodeImpl<RVOpcode>;
  };
  struct UTypeFields {
    using Rd = RegRd<0>;
    using Imm = ImmU<symbolType>;
    using Impl = FieldsImpl<Rd, Imm>;
  };

  using Opcode = UTypeOpcode;
  using Fields = UTypeFields;
};

struct Auipc
    : public Instr<Auipc, RVISA::OpcodeID::AUIPC, SymbolType::Absolute> {
  constexpr static std::string_view Name = "auipc";
};

struct Lui : public Instr<Lui, RVISA::OpcodeID::LUI> {
  constexpr static std::string_view Name = "lui";
};

} // namespace TypeU

namespace TypeJ {

/// A RISC-V signed immediate field with an input width of 21 bits.
/// Used in J-Type instructions.
///
/// It is defined as:
///  - Imm[31:20] = Inst[31]
///  - Imm[19:12] = Inst[19:12]
///  - Imm[11]    = Inst[20]
///  - Imm[10:5]  = Inst[30:25]
///  - Imm[4:1]   = Inst[24:21]
///  - Imm[0]     = 0
struct ImmJ
    : public ImmSym<1, 21, Repr::Signed,
                    ImmPartsImpl<ImmPart<20, 31, 31>, ImmPart<12, 12, 19>,
                                 ImmPart<11, 20, 20>, ImmPart<1, 21, 30>>,
                    SymbolType::Relative> {};

struct Jal : public RV_Instruction<Jal> {
  struct JTypeOpcode {
    using RVOpcode = OpPartOpcode<static_cast<unsigned>(RVISA::OpcodeID::JAL)>;
    using Impl = OpcodeImpl<RVOpcode>;
  };
  struct JTypeFields {
    using Rd = RegRd<0>;
    using Imm = ImmJ;
    using Impl = FieldsImpl<Rd, Imm>;
  };

  using Opcode = JTypeOpcode;
  using Fields = JTypeFields;

  constexpr static std::string_view Name = "jal";
};

} // namespace TypeJ

namespace TypeS {

enum class Funct3 { SB = 0b000, SH = 0b001, SW = 0b010, SD = 0b011 };

/// A RISC-V signed immediate field with an input width of 12 bits.
/// Used in S-Type instructions.
///
/// It is defined as:
///  - Imm[31:11] = Inst[31]
///  - Imm[10:5]  = Inst[30:25]
///  - Imm[4:0]   = Inst[11:7]
struct ImmS : public Imm<1, 12, Repr::Signed,
                         ImmPartsImpl<ImmPart<5, 25, 31>, ImmPart<0, 7, 11>>> {
};

template <typename InstrImpl, Funct3 funct3>
struct Instr : public RV_Instruction<InstrImpl> {
  struct STypeOpcode {
    using RVOpcode =
        OpPartOpcode<static_cast<unsigned>(RVISA::OpcodeID::STORE)>;
    using RVFunct3 = OpPartFunct3<static_cast<unsigned>(funct3)>;
    using Impl = OpcodeImpl<RVOpcode, RVFunct3>;
  };
  struct STypeFields {
    using Rs2 = RegRs2<0>;
    using Imm = ImmS;
    using Rs1 = RegRs1<2>;
    using Impl = FieldsImpl<Rs2, Imm, Rs1>;
  };

  using Opcode = STypeOpcode;
  using Fields = STypeFields;
};

struct Sb : public Instr<Sb, Funct3::SB> {
  constexpr static std::string_view Name = "sb";
};

struct Sw : public Instr<Sw, Funct3::SW> {
  constexpr static std::string_view Name = "sw";
};

struct Sh : public Instr<Sh, Funct3::SH> {
  constexpr static std::string_view Name = "sh";
};

struct Sd : public Instr<Sd, Funct3::SD> {
  constexpr static std::string_view Name = "sd";
};

} // namespace TypeS

namespace TypeR {

enum class Funct3 {
  ADD = 0b000,
  SUB = 0b000,
  SLL = 0b001,
  SLT = 0b010,
  SLTU = 0b011,
  XOR = 0b100,
  SRL = 0b101,
  SRA = 0b101,
  OR = 0b110,
  AND = 0b111
};
enum class Funct7 { DEFAULT = 0b0000000, SUB_SRA = 0b0100000 };

template <typename InstrImpl, RVISA::OpcodeID opcode, Funct3 funct3,
          Funct7 funct7>
struct Instr : public RV_Instruction<InstrImpl> {
  struct RTypeOpcode {
    using RVOpcode = OpPartOpcode<static_cast<unsigned>(opcode)>;
    using RVFunct3 = OpPartFunct3<static_cast<unsigned>(funct3)>;
    using RVFunct7 = OpPartFunct7<static_cast<unsigned>(funct7)>;
    using Impl = OpcodeImpl<RVOpcode, RVFunct3, RVFunct7>;
  };
  struct RTypeFields {
    using Rd = RegRd<0>;
    using Rs1 = RegRs1<1>;
    using Rs2 = RegRs2<2>;
    using Impl = FieldsImpl<Rd, Rs1, Rs2>;
  };

  using Opcode = RTypeOpcode;
  using Fields = RTypeFields;
};

template <typename InstrImpl, Funct3 funct3, Funct7 funct7 = Funct7::DEFAULT>
using Instr32 = Instr<InstrImpl, OpcodeID::OP, funct3, funct7>;
template <typename InstrImpl, Funct3 funct3, Funct7 funct7 = Funct7::DEFAULT>
using Instr64 = Instr<InstrImpl, OpcodeID::OP32, funct3, funct7>;

struct Add : public Instr32<Add, Funct3::ADD> {
  constexpr static std::string_view Name = "add";
};

struct Sub : public Instr32<Sub, Funct3::SUB, Funct7::SUB_SRA> {
  constexpr static std::string_view Name = "sub";
};

struct Sll : public Instr32<Sll, Funct3::SLL> {
  constexpr static std::string_view Name = "sll";
};

struct Slt : public Instr32<Slt, Funct3::SLT> {
  constexpr static std::string_view Name = "slt";
};

struct Sltu : public Instr32<Sltu, Funct3::SLTU> {
  constexpr static std::string_view Name = "sltu";
};

struct Xor : public Instr32<Xor, Funct3::XOR> {
  constexpr static std::string_view Name = "xor";
};

struct Srl : public Instr32<Srl, Funct3::SRL> {
  constexpr static std::string_view Name = "srl";
};

struct Sra : public Instr32<Sra, Funct3::SRA, Funct7::SUB_SRA> {
  constexpr static std::string_view Name = "sra";
};

struct Or : public Instr32<Or, Funct3::OR> {
  constexpr static std::string_view Name = "or";
};

struct And : public Instr32<And, Funct3::AND> {
  constexpr static std::string_view Name = "and";
};

struct Addw : public Instr64<Addw, Funct3::ADD> {
  constexpr static std::string_view Name = "addw";
};

struct Subw : public Instr64<Subw, Funct3::SUB, Funct7::SUB_SRA> {
  constexpr static std::string_view Name = "subw";
};

struct Sllw : public Instr64<Sllw, Funct3::SLL> {
  constexpr static std::string_view Name = "sllw";
};

struct Srlw : public Instr64<Srlw, Funct3::SRL> {
  constexpr static std::string_view Name = "srlw";
};

struct Sraw : public Instr64<Sraw, Funct3::SRA, Funct7::SUB_SRA> {
  constexpr static std::string_view Name = "sraw";
};

} // namespace TypeR

namespace TypeB {

enum class Funct3 {
  BEQ = 0b000,
  BNE = 0b001,
  BLT = 0b100,
  BGE = 0b101,
  BLTU = 0b110,
  BGEU = 0b111
};

/// A RISC-V signed immediate field with an input width of 13 bits.
/// Used in B-Type instructions.
///
/// It is defined as:
///  - Imm[31:12] = Inst[31]
///  - Imm[11]    = Inst[7]
///  - Imm[10:5]  = Inst[30:25]
///  - Imm[4:1]   = Inst[11:8]
///  - Imm[0]     = 0
struct ImmB : public ImmSym<2, 13, Repr::Signed,
                            ImmPartsImpl<ImmPart<12, 31, 31>, ImmPart<11, 7, 7>,
                                         ImmPart<5, 25, 30>, ImmPart<1, 8, 11>>,
                            SymbolType::Relative> {};

template <typename InstrImpl, Funct3 funct3>
struct Instr : public RV_Instruction<InstrImpl> {
  struct BTypeOpcode {
    using RVOpcode =
        OpPartOpcode<static_cast<unsigned>(RVISA::OpcodeID::BRANCH)>;
    using RVFunct3 = OpPartFunct3<static_cast<unsigned>(funct3)>;
    using Impl = OpcodeImpl<RVOpcode, RVFunct3>;
  };
  struct BTypeFields {
    using Rs1 = RegRs1<0>;
    using Rs2 = RegRs2<1>;
    using Imm = ImmB;
    using Impl = FieldsImpl<Rs1, Rs2, ImmB>;
  };

  using Opcode = BTypeOpcode;
  using Fields = BTypeFields;
};

struct Beq : public Instr<Beq, Funct3::BEQ> {
  constexpr static std::string_view Name = "beq";
};

struct Bne : public Instr<Bne, Funct3::BNE> {
  constexpr static std::string_view Name = "bne";
};

struct Blt : public Instr<Blt, Funct3::BLT> {
  constexpr static std::string_view Name = "blt";
};

struct Bge : public Instr<Bge, Funct3::BGE> {
  constexpr static std::string_view Name = "bge";
};

struct Bltu : public Instr<Bltu, Funct3::BLTU> {
  constexpr static std::string_view Name = "bltu";
};

struct Bgeu : public Instr<Bgeu, Funct3::BGEU> {
  constexpr static std::string_view Name = "bgeu";
};

} // namespace TypeB

namespace TypePseudo {

struct Lb : public PseudoInstrLoad<Lb> {
  constexpr static std::string_view Name = "lb";
};
struct Lh : public PseudoInstrLoad<Lh> {
  constexpr static std::string_view Name = "lh";
};
struct Lw : public PseudoInstrLoad<Lw> {
  constexpr static std::string_view Name = "lw";
};

struct La : public PseudoInstruction<La> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };

  static Result<std::vector<LineTokens>> expander(const PseudoInstruction<La> &,
                                                  const TokenizedSrcLine &line,
                                                  const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens() << Token("auipc") << line.tokens.at(1)
                             << Token(line.tokens.at(2), "%pcrel_hi"));
    v.push_back(LineTokens()
                << Token("addi") << line.tokens.at(1) << line.tokens.at(1)
                << Token(QString("(%1 + 4)").arg(line.tokens.at(2)),
                         "%pcrel_lo"));
    return v;
  }
  constexpr static std::string_view Name = "la";
};

struct Call : public PseudoInstruction<Call> {
  struct Fields {
    using Imm = PseudoImm<0>;
    using Impl = FieldsImpl<Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Call> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens() << Token("auipc") << Token("x1")
                             << Token(line.tokens.at(1), "%pcrel_hi"));
    v.push_back(LineTokens()
                << Token("jalr") << Token("x1") << Token("x1")
                << Token(QString("(%1 + 4)").arg(line.tokens.at(1)),
                         "%pcrel_lo"));
    return v;
  }
  constexpr static std::string_view Name = "call";
};

struct Tail : public PseudoInstruction<Tail> {
  struct Fields {
    using Imm = PseudoImm<0>;
    using Impl = FieldsImpl<Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Tail> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens() << Token("auipc") << Token("x6")
                             << Token(line.tokens.at(1), "%pcrel_hi"));
    v.push_back(LineTokens()
                << Token("jalr") << Token("x0") << Token("x6")
                << Token(QString("(%1 + 4)").arg(line.tokens.at(1)),
                         "%pcrel_lo"));
    return v;
  }
  constexpr static std::string_view Name = "tail";
};

struct J : public PseudoInstruction<J> {
  struct Fields {
    using Imm = PseudoImm<0>;
    using Impl = FieldsImpl<Imm>;
  };

  static Result<std::vector<LineTokens>> expander(const PseudoInstruction<J> &,
                                                  const TokenizedSrcLine &line,
                                                  const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens()
                << Token("jal") << Token("x0") << line.tokens.at(1));
    return v;
  }
  constexpr static std::string_view Name = "j";
};

struct Jr : public PseudoInstruction<Jr> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Impl = FieldsImpl<Reg>;
  };

  static Result<std::vector<LineTokens>> expander(const PseudoInstruction<Jr> &,
                                                  const TokenizedSrcLine &line,
                                                  const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens() << Token("jalr") << Token("x0")
                             << line.tokens.at(1) << Token("0"));
    return v;
  }
  constexpr static std::string_view Name = "jr";
};

struct Jalr : public PseudoInstruction<Jalr> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Impl = FieldsImpl<Reg>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Jalr> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens() << Token("jalr") << Token("x1")
                             << line.tokens.at(1) << Token("0"));
    return v;
  }
  constexpr static std::string_view Name = "jalr";
};

struct Ret : public PseudoInstruction<Ret> {
  struct Fields {
    using Impl = FieldsImpl<>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Ret> &, const TokenizedSrcLine &,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens()
                << Token("jalr") << Token("x0") << Token("x1") << Token("0"));
    return v;
  }
  constexpr static std::string_view Name = "ret";
};

struct Jal : public PseudoInstruction<Jal> {
  struct Fields {
    using Imm = PseudoImm<0>;
    using Impl = FieldsImpl<Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Jal> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens()
                << Token("jal") << Token("x1") << line.tokens.at(1));
    return v;
  }
  constexpr static std::string_view Name = "jal";
};

struct Nop : public PseudoInstruction<Nop> {
  struct Fields {
    using Impl = FieldsImpl<>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Nop> &, const TokenizedSrcLine &,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens()
                << Token("addi") << Token("x0") << Token("x0") << Token("0"));
    return v;
  }
  constexpr static std::string_view Name = "nop";
};

struct Mv : public PseudoInstruction<Mv> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Impl = FieldsImpl<Reg0, Reg1>;
  };

  static Result<std::vector<LineTokens>> expander(const PseudoInstruction<Mv> &,
                                                  const TokenizedSrcLine &line,
                                                  const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("addi"), line.tokens.at(1), line.tokens.at(2),
                           Token("0")});
    return v;
  }
  constexpr static std::string_view Name = "mv";
};

struct Not : public PseudoInstruction<Not> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Impl = FieldsImpl<Reg0, Reg1>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Not> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("xori"), line.tokens.at(1), line.tokens.at(2),
                           Token("-1")});
    return v;
  }
  constexpr static std::string_view Name = "not";
};

struct Neg : public PseudoInstruction<Neg> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Impl = FieldsImpl<Reg0, Reg1>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Neg> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("sub"), line.tokens.at(1), Token("x0"),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "neg";
};

struct Seqz : public PseudoInstruction<Seqz> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Impl = FieldsImpl<Reg0, Reg1>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Seqz> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("sltiu"), line.tokens.at(1), line.tokens.at(2),
                           Token("1")});
    return v;
  }
  constexpr static std::string_view Name = "seqz";
};

struct Snez : public PseudoInstruction<Snez> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Impl = FieldsImpl<Reg0, Reg1>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Snez> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("sltu"), line.tokens.at(1), Token("x0"),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "snez";
};

struct Sltz : public PseudoInstruction<Sltz> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Impl = FieldsImpl<Reg0, Reg1>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Sltz> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("slt"), line.tokens.at(1), line.tokens.at(2),
                           Token("x0")});
    return v;
  }
  constexpr static std::string_view Name = "sltz";
};

struct Sgtz : public PseudoInstruction<Sgtz> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Impl = FieldsImpl<Reg0, Reg1>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Sgtz> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("slt"), line.tokens.at(1), Token("x0"),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "sgtz";
};

struct Beqz : public PseudoInstruction<Beqz> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Beqz> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("beq"), line.tokens.at(1), Token("x0"),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "beqz";
};

struct Bnez : public PseudoInstruction<Bnez> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Bnez> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("bne"), line.tokens.at(1), Token("x0"),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "bnez";
};

struct Blez : public PseudoInstruction<Blez> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Blez> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("bge"), Token("x0"), line.tokens.at(1),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "blez";
};

struct Bgez : public PseudoInstruction<Bgez> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Bgez> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("bge"), line.tokens.at(1), Token("x0"),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "bgez";
};

struct Bltz : public PseudoInstruction<Bltz> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Bltz> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("blt"), line.tokens.at(1), Token("x0"),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "bltz";
};

struct Bgtz : public PseudoInstruction<Bgtz> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Bgtz> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("blt"), Token("x0"), line.tokens.at(1),
                           line.tokens.at(2)});
    return v;
  }
  constexpr static std::string_view Name = "bgtz";
};

struct Bgt : public PseudoInstruction<Bgt> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Imm = PseudoImm<2>;
    using Impl = FieldsImpl<Reg0, Reg1, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Bgt> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("blt"), line.tokens.at(2), line.tokens.at(1),
                           line.tokens.at(3)});
    return v;
  }
  constexpr static std::string_view Name = "bgt";
};

struct Ble : public PseudoInstruction<Ble> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Imm = PseudoImm<2>;
    using Impl = FieldsImpl<Reg0, Reg1, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Ble> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("bge"), line.tokens.at(2), line.tokens.at(1),
                           line.tokens.at(3)});
    return v;
  }
  constexpr static std::string_view Name = "ble";
};

struct Bgtu : public PseudoInstruction<Bgtu> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Imm = PseudoImm<2>;
    using Impl = FieldsImpl<Reg0, Reg1, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Bgtu> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("bltu"), line.tokens.at(2), line.tokens.at(1),
                           line.tokens.at(3)});
    return v;
  }
  constexpr static std::string_view Name = "bgtu";
};

struct Bleu : public PseudoInstruction<Bleu> {
  struct Fields {
    using Reg0 = PseudoReg<0>;
    using Reg1 = PseudoReg<1>;
    using Imm = PseudoImm<2>;
    using Impl = FieldsImpl<Reg0, Reg1, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Bleu> &, const TokenizedSrcLine &line,
           const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens{Token("bgeu"), line.tokens.at(2), line.tokens.at(1),
                           line.tokens.at(3)});
    return v;
  }
  constexpr static std::string_view Name = "bleu";
};

template <bool isRV64>
struct Li : public PseudoInstruction<Li<isRV64>> {
  struct Fields {
    using Reg = PseudoReg<0>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };

  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<Li<isRV64>> &, const TokenizedSrcLine &line,
           const SymbolMap &symbols) {
    LineTokensVec res;
    // Get an integer representation of the immediate, which might have
    // been a symbol.
    bool canConvert;
    bool liveDstReg = false;
    bool unsignedFitErr = false;
    int64_t immediate = getImmediateSext32(line.tokens.at(2), canConvert);

    if (!canConvert) {
      auto absSymbols = symbols.copyRelativeTo(line.sourceLine());
      // Check if the immediate has been made available in the symbol set
      // at this point...
      auto it = absSymbols.find(line.tokens.at(2));
      if (it != absSymbols.end()) {
        immediate = it->second;
      } else {
        if (unsignedFitErr) {
          return Result<std::vector<LineTokens>>{
              Error(line, QString("Invalid immediate '%1'; can't emit "
                                  ">32-bit imm for non-RV64 target")
                              .arg(line.tokens.at(2)))};
        } else {
          return Result<std::vector<LineTokens>>{Error(
              line, QString("Invalid immediate '%1'").arg(line.tokens.at(2)))};
        }
      }
    }

    /* The load-immediate pseudo instructions follows the LLVM
     * implementation, as seen here.
     * https://llvm.org/docs/doxygen/RISCVMatInt_8cpp_source.html
     * For more insight, please refer to there, since their comments have
     * been left out of this source code.
     */
    std::function<Result<std::vector<LineTokens>>(int64_t)> genInstrSeq =
        [&](int64_t val) {
          if (isInt<32>(val) || (!isRV64 && isUInt<32>(val))) {
            int64_t Hi20 = ((val + 0x800) >> 12) & 0xFFFFF;
            int64_t Lo12 = vsrtl::signextend<12>(val);
            if (Hi20) {
              res.push_back(LineTokens() << Token("lui") << line.tokens.at(1)
                                         << QString::number(Hi20));
              liveDstReg = true;
            }
            if (Lo12 || Hi20 == 0) {
              QString addiOpc = isRV64 && Hi20 != 0 ? "addiw" : "addi";
              res.push_back(LineTokens()
                            << Token(addiOpc) << line.tokens.at(1)
                            << (liveDstReg ? line.tokens.at(1) : Token("x0"))
                            << QString::number(Lo12));
              liveDstReg = true;
            }
            return Result<std::vector<LineTokens>>{res};
          }
          if (!isRV64) {
            return Result<std::vector<LineTokens>>{
                Error(line, QString("Invalid immediate '%1'; can't emit "
                                    ">32-bit imm for non-RV64 target")
                                .arg(line.tokens.at(2)))};
          }
          int64_t Lo12 = vsrtl::signextend<12>(val);
          int64_t Hi52 = ((VInt)val + 0x800ull) >> 12;
          int ShiftAmount = 12 + firstSetBitIdx(Hi52);
          Hi52 = vsrtl::signextend<int64_t>(Hi52 >> (ShiftAmount - 12),
                                            64 - ShiftAmount);
          genInstrSeq(Hi52);
          res.push_back(LineTokens()
                        << Token("slli") << line.tokens.at(1)
                        << line.tokens.at(1) << QString::number(ShiftAmount));
          if (Lo12) {
            res.push_back(LineTokens()
                          << Token("addi") << line.tokens.at(1)
                          << line.tokens.at(1) << QString::number(Lo12));
          }
          return Result<std::vector<LineTokens>>{res};
        };
    auto instrSeq = genInstrSeq(immediate);
    return instrSeq;
  }
  constexpr static std::string_view Name = "li";
};

using Li32 = Li<false>;
using Li64 = Li<true>;

} // namespace TypePseudo

template <typename InstrVecType, typename... Instructions>
constexpr inline static void _enableInstructions(InstrVecType &instructions) {
  (instructions.emplace_back(std::make_unique<Instructions>()), ...);
}

template <typename... Instructions>
constexpr inline static void enableInstructions(InstrVec &instructions) {
  static_assert((InstrVerify<Instructions>::IsVerified && ...),
                "Could not verify instruction");
  // TODO: Ensure no duplicate instruction definitions
  return _enableInstructions<InstrVec, Instructions...>(instructions);
}

template <typename... Instructions>
constexpr inline static void
enablePseudoInstructions(PseudoInstrVec &instructions) {
  // TODO: Ensure no duplicate pseudo-instruction definitions
  // TODO: Verify pseudoinstructions
  // TODO: Verify instructions generated from pseudoinstructions
  return _enableInstructions<PseudoInstrVec, Instructions...>(instructions);
}

}; // namespace ExtI

} // namespace RVISA
} // namespace Ripes
