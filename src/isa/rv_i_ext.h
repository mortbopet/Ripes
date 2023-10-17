#pragma once

#include <bitset>
#include <iostream>

#include "pseudoinstruction.h"
#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

struct RVIExt {
  /// A RISC-V signed immediate field with a width of 12 bits.
  /// Used in L-Type and I-Type instructions.
  ///
  /// It is defined as:
  ///  - Imm[31:11] = Inst[31]
  ///  - Imm[10:0]  = Inst[30:20]
  template <unsigned tokenIndex>
  struct ImmCommon12 : public Imm<tokenIndex, 12, Repr::Signed,
                                  ImmPartsImpl<ImmPart<0, BitRange<20, 31>>>> {
  };

  /// A base I-Type RISC-V instruction
  template <typename InstrImpl, unsigned RVOpcodeID>
  struct InstrITypeBase : public Instruction<InstrImpl> {
    struct ITypeOpcode {
      using RVOpcode = OpPartOpcode<RVOpcodeID>;
      using RVFunct3 = OpPartFunct3<InstrImpl::funct3()>;
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

  /// A I-Type RISC-V instruction
  template <typename InstrImpl>
  struct InstrIType : public InstrITypeBase<InstrImpl, OpcodeID::OPIMM> {
    enum Funct3ID {
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

    using Opcode = typename InstrITypeBase<InstrImpl, OpcodeID::OPIMM>::Opcode;
    using Fields = typename InstrITypeBase<InstrImpl, OpcodeID::OPIMM>::Fields;
  };

  struct Lb : public PseudoInstrLoad<Lb> {
    static QString mnemonic() { return "lb"; }
  };
  struct Lh : public PseudoInstrLoad<Lh> {
    static QString mnemonic() { return "lh"; }
  };
  struct Lw : public PseudoInstrLoad<Lw> {
    static QString mnemonic() { return "lw"; }
  };

  struct AddI : public InstrIType<AddI> {
    static QString mnemonic() { return "addi"; }
    constexpr static unsigned funct3() {
      return InstrIType<AddI>::Funct3ID::ADDI;
    }
    using Opcode = typename InstrIType<AddI>::Opcode;
    using Fields = typename InstrIType<AddI>::Fields;
  };

  std::vector<std::unique_ptr<InstructionBase>> instructions;
  std::vector<std::unique_ptr<PseudoInstructionBase>> pseudoInstructions;

  RVIExt() {
    pseudoInstructions.emplace_back(std::make_unique<Lb>());
    pseudoInstructions.emplace_back(std::make_unique<Lh>());
    pseudoInstructions.emplace_back(std::make_unique<Lw>());

    instructions.emplace_back(std::make_unique<AddI>());
  }
};

struct RV32ISA : public RVIExt {
  // TODO: Link this into the assembler
  AssembleRes assemble(const TokenizedSrcLine &tokens) {
    return this->instructions.at(0)->assemble(tokens);
  }
};

} // namespace RVISA
} // namespace Ripes
