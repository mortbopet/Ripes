#pragma once

#include <bitset>
#include <iostream>

#include "pseudoinstruction.h"
#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

namespace ExtI {
/// A RISC-V signed immediate field with a width of 12 bits.
/// Used in L-Type and I-Type instructions.
///
/// It is defined as:
///  - Imm[31:11] = Inst[31]
///  - Imm[10:0]  = Inst[30:20]
template <unsigned tokenIndex>
struct ImmCommon12 : public Imm<tokenIndex, 12, Repr::Signed,
                                ImmPartsImpl<ImmPart<0, BitRange<20, 31>>>> {};

namespace TypeI {

/// A base I-Type RISC-V instruction
template <typename InstrImpl, unsigned OpcodeID, unsigned Funct3>
struct Base : public Instruction<InstrImpl> {
  struct ITypeOpcode {
    using RVOpcode = OpPartOpcode<OpcodeID>;
    using RVFunct3 = OpPartFunct3<Funct3>;
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

enum ITypeFunct3 : unsigned {
  ADDI = 0b000,
  SLTI = 0b010,
  SLTIU = 0b011,
  XORI = 0b100,
  ORI = 0b110,
  ANDI = 0b111,
  SLLI = 0b001,
  SRLI = 0b101,
  SRAI = 0b101
};

/// A I-Type RISC-V instruction
template <typename InstrImpl, ITypeFunct3 funct3>
struct Instr : public Base<InstrImpl, OpcodeID::OPIMM, funct3> {};

struct AddI : public Instr<AddI, ITypeFunct3::ADDI> {
  static QString mnemonic() { return "addi"; }
};

struct AndI : public Instr<AndI, ITypeFunct3::ANDI> {
  static QString mnemonic() { return "andi"; }
};

} // namespace TypeI

struct Lb : public PseudoInstrLoad<Lb> {
  static QString mnemonic() { return "lb"; }
};
struct Lh : public PseudoInstrLoad<Lh> {
  static QString mnemonic() { return "lh"; }
};
struct Lw : public PseudoInstrLoad<Lw> {
  static QString mnemonic() { return "lw"; }
};

template <typename InstrVecType, typename... Instructions>
inline static void _enableInstructions(InstrVecType &instructions) {
  (instructions.emplace_back(std::make_unique<Instructions>()), ...);
}

template <typename... Instructions>
inline static void enableInstructions(InstrVec &instructions) {
  return _enableInstructions<InstrVec, Instructions...>(instructions);
}

template <typename... Instructions>
inline static void enablePseudoInstructions(PseudoInstrVec &instructions) {
  return _enableInstructions<PseudoInstrVec, Instructions...>(instructions);
}

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);

}; // namespace ExtI

} // namespace RVISA
} // namespace Ripes
