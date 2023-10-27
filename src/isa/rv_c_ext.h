#pragma once

#include "pseudoinstruction.h"
#include "rv_i_ext.h"
#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

namespace ExtC {

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);

constexpr unsigned INSTR_BITS = 16;

template <typename InstrImpl>
struct RVC_Instruction : public Instruction<InstrImpl> {
  constexpr static unsigned InstrBits() { return INSTR_BITS; }
};

enum class Funct2Offset { OFFSET5 = 5, OFFSET10 = 10 };

// TODO: Split this into 2 classes?
/// All RISC-V Funct2 opcode parts are defined as a 2-bit field in bits 5-6 or
/// 10-11 of the instruction
template <unsigned funct2, Funct2Offset offset = Funct2Offset::OFFSET5>
struct OpPartFunct2
    : public OpPart<static_cast<unsigned>(funct2),
                    BitRange<static_cast<unsigned>(offset),
                             static_cast<unsigned>(offset) + 1, INSTR_BITS>> {};

/// The RV-C Rs2' field contains a source register index.
/// It is defined as a 3-bit field in bits 2-4 of the instruction
template <unsigned tokenIndex>
struct RegRs2Prime : public GPR_Reg<RegRs2Prime<tokenIndex>, tokenIndex,
                                    BitRange<2, 4, INSTR_BITS>> {
  constexpr static std::string_view Name = "rs2'";
};

namespace TypeCA {

enum class Funct2 { SUB = 0b00, XOR_ADD = 0b01, OR = 0b10, AND = 0b11 };
enum class Funct6 { DEFAULT = 0b100011, WIDE = 0b100111 };

/// All RV-C Funct6 opcode parts are defined as a 6-bit field in bits 10-15
/// of the Funct6
template <Funct6 funct6>
struct OpPartFunct6 : public OpPart<static_cast<unsigned>(funct6),
                                    BitRange<10, 15, INSTR_BITS>> {};

// TODO: Maybe make a base class for RVC registers to add extra verifications
/// The RV-C Rd'/Rs1' field contains a source or destination
/// register index.
/// It is defined as a 3-bit field in bits 7-9 of the instruction
template <unsigned tokenIndex>
struct RegRdRs1Prime : public GPR_Reg<RegRdRs1Prime<tokenIndex>, tokenIndex,
                                      BitRange<7, 9, INSTR_BITS>> {
  constexpr static std::string_view Name = "rd'/rs1'";
};

template <typename InstrImpl, Funct2 funct2, Funct6 funct6 = Funct6::DEFAULT>
struct Instr : public RVC_Instruction<InstrImpl> {
  struct CATypeOpcode {
    using RVQuadrant =
        OpPartQuadrant<static_cast<unsigned>(QuadrantID::QUADRANT1),
                       INSTR_BITS>;
    using RVFunct2 = OpPartFunct2<static_cast<unsigned>(funct2)>;
    using RVFunct6 = OpPartFunct6<funct6>;
    using Impl = OpcodeImpl<RVQuadrant, RVFunct2, RVFunct6>;
  };
  struct CATypeFields {
    using Reg0 = RegRdRs1Prime<0>;
    using Reg1 = RegRs2Prime<1>;
    using Impl = FieldsImpl<Reg0, Reg1>;
  };

  using Opcode = CATypeOpcode;
  using Fields = CATypeFields;
};

struct CSub : Instr<CSub, Funct2::SUB> {
  constexpr static std::string_view Name = "c.sub";
};

struct CXor : Instr<CXor, Funct2::XOR_ADD> {
  constexpr static std::string_view Name = "c.xor";
};

struct COr : Instr<COr, Funct2::OR> {
  constexpr static std::string_view Name = "c.or";
};

struct CAnd : Instr<CAnd, Funct2::AND> {
  constexpr static std::string_view Name = "c.and";
};

struct CSubw : Instr<CSubw, Funct2::SUB, Funct6::WIDE> {
  constexpr static std::string_view Name = "c.subw";
};

struct CAddw : Instr<CAddw, Funct2::XOR_ADD, Funct6::WIDE> {
  constexpr static std::string_view Name = "c.addw";
};

} // namespace TypeCA

} // namespace ExtC

} // namespace RVISA
} // namespace Ripes
