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
struct Instr : public Instruction<InstrImpl> {
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
  static QString mnemonic() { return "addi"; }
};

struct Andi : public Instr32<Andi, Funct3::ANDI> {
  static QString mnemonic() { return "andi"; }
};

struct Slti : public Instr32<Slti, Funct3::SLTI> {
  static QString mnemonic() { return "slti"; }
};

struct Sltiu : public Instr32<Sltiu, Funct3::SLTIU> {
  static QString mnemonic() { return "sltiu"; }
};

struct Xori : public Instr32<Xori, Funct3::XORI> {
  static QString mnemonic() { return "xori"; }
};

struct Ori : public Instr32<Ori, Funct3::ORI> {
  static QString mnemonic() { return "ori"; }
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
struct ImmIShift32 : public Imm<ImmTokenIndex, 5, Repr::Unsigned,
                                ImmPart<0, BitRange<20, 24>>> {};

/// An IShift-Type RISC-V instruction
template <typename InstrImpl, OpcodeID opcodeID, Funct3 funct3, Funct7 funct7>
struct Instr : public Instruction<InstrImpl> {
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

template <typename InstrImpl, Funct3 funct3, Funct7 funct7>
using Instr32 = Instr<InstrImpl, OpcodeID::OPIMM, funct3, funct7>;
template <typename InstrImpl, Funct3 funct3, Funct7 funct7>
using Instr64 = Instr<InstrImpl, OpcodeID::OPIMM32, funct3, funct7>;

struct Slli : public Instr32<Slli, Funct3::SLLI, Funct7::LEFT_SHIFT> {
  static QString mnemonic() { return "slli"; }
};

struct Srli : public Instr32<Srli, Funct3::SRLI, Funct7::LEFT_SHIFT> {
  static QString mnemonic() { return "srli"; }
};

struct Srai : public Instr32<Srai, Funct3::SRAI, Funct7::RIGHT_SHIFT> {
  static QString mnemonic() { return "srai"; }
};

} // namespace TypeIShift

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
  // TODO: Ensure no duplicate instruction definitions
  return _enableInstructions<InstrVec, Instructions...>(instructions);
}

template <typename... Instructions>
inline static void enablePseudoInstructions(PseudoInstrVec &instructions) {
  // TODO: Ensure no duplicate pseudo-instruction definitions
  return _enableInstructions<PseudoInstrVec, Instructions...>(instructions);
}

enum class Options {
  shifts64BitVariant, // appends 'w' to 32-bit shift operations, for use in
                      // the 64-bit RISC-V ISA
  LI64BitVariant      // Modifies LI to be able to emit 64-bit constants
};

void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions,
               const std::set<Options> &options = {});

}; // namespace ExtI

} // namespace RVISA
} // namespace Ripes
