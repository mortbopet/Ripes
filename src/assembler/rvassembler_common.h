#pragma once

#include "../isa/rvisainfo_common.h"
#include "assembler.h"

namespace Ripes {
namespace Assembler {

/// A base class for all RISC-V opcode parts
struct RVOpPart : public OpPart {
  RVOpPart(unsigned value, unsigned start, unsigned stop)
      : OpPart(value, start, stop) {}
};

/// All RISC-V opcodes are defined as a 7-bit field in bits 0-7 of the
/// instruction
struct RVOpPartOpcode : public RVOpPart {
  RVOpPartOpcode(RVISA::Opcode opcode) : RVOpPart(opcode, 0, 6) {}
};

/// All RISC-V instruction quadrants are defined as a 2-bit field in bits 0-1 of
/// the instruction
struct RVOpPartQuadrant : public RVOpPart {
  RVOpPartQuadrant(RVISA::Quadrant quadrant) : RVOpPart(quadrant, 0, 1) {}
};

/// All RISC-V Funct3 opcode parts are defined as a 3-bit field in bits 12-14 of
/// the instruction
struct RVOpPartFunct3 : public RVOpPart {
  RVOpPartFunct3(unsigned funct3) : RVOpPart(funct3, 12, 14) {}
};

/// All RISC-V Funct6 opcode parts are defined as a 6-bit field in bits 26-31 of
/// the instruction
struct RVOpPartFunct6 : public RVOpPart {
  RVOpPartFunct6(unsigned funct6) : RVOpPart(funct6, 26, 31) {}
};

/// All RISC-V Funct7 opcode parts are defined as a 7-bit field in bits 25-31 of
/// the instruction
struct RVOpPartFunct7 : public RVOpPart {
  RVOpPartFunct7(unsigned funct7) : RVOpPart(funct7, 25, 31) {}
};

/// A RISC-V opcode defines the encoding of specific instructions
struct RVOpcode : public Opcode {
  /// Construct opcode from parts
  RVOpcode(const Token &name, const std::vector<OpPart> &opParts)
      : Opcode(name, opParts) {}

  /// A RISC-V opcode with no parts
  RVOpcode(const Token &name, RVISA::Opcode opcode)
      : Opcode(name, {RVOpPartOpcode(opcode)}) {}

  /// A RISC-V opcode with a Funct3 part
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3)
      : Opcode(name, {RVOpPartOpcode(opcode), funct3}) {}

  /// A RISC-V opcode with Funct3 and Funct6 parts
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3,
           RVOpPartFunct6 funct6)
      : Opcode(name, {RVOpPartOpcode(opcode), funct3, funct6}) {}

  /// A RISC-V opcode with Funct3 and Funct7 parts
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3,
           RVOpPartFunct7 funct7)
      : Opcode(name, {RVOpPartOpcode(opcode), funct3, funct7}) {}
};

/// A base class for all RISC-V instructions
struct RVInstructionBase : public Instruction {
  RVInstructionBase(const RVOpcode &opcode,
                    const std::vector<std::shared_ptr<Field>> &fields)
      : Instruction(opcode, fields) {}
};

/// RISC-V instruction class (for compressed instructions, see class
/// RVCInstruction)
struct RVInstruction : public RVInstructionBase {
  RVInstruction(const RVOpcode &opcode,
                const std::vector<std::shared_ptr<Field>> &fields)
      : RVInstructionBase(opcode, fields) {}
};

/// A base class for all RISC-V register types
struct RVReg : public Reg {
  RVReg(const ISAInfoBase *isa, unsigned tokenIndex, unsigned start,
        unsigned stop, const QString &regsd)
      : Reg(isa, tokenIndex, start, stop, regsd) {}
};

/// The RISC-V Rs1 field contains a source register index.
/// It is defined as a 5-bit field in bits 15-19 of the instruction
struct RVRegRs1 : public RVReg {
  RVRegRs1(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg(isa, fieldIndex, 15, 19, "rs1") {}
};

/// The RISC-V Rs2 field contains a source register index.
/// It is defined as a 5-bit field in bits 20-24 of the instruction
struct RVRegRs2 : public RVReg {
  RVRegRs2(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg(isa, fieldIndex, 20, 24, "rs2") {}
};

/// The RISC-V Rd field contains a destination register index.
/// It is defined as a 5-bit field in bits 7-11 of the instruction
struct RVRegRd : public RVReg {
  RVRegRd(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg(isa, fieldIndex, 7, 11, "rd") {}
};

/// A base class for all RISC-V immediate types
struct RVImm : public Imm {
  RVImm(unsigned tokenIndex, unsigned width, typename Imm::Repr repr,
        const std::vector<ImmPart> &parts,
        typename Imm::SymbolType symbolType = Imm::SymbolType::None)
      : Imm(tokenIndex, width, repr, parts, symbolType) {}
};

/// A RISC-V signed immediate field with an input width of 12 bits.
/// Used in L-Type and I-Type instructions.
///
/// It is defined as:
///  - Imm[31:11] = Inst[31]
///  - Imm[10:0]  = Inst[30:20]
struct RVImmI : public RVImm {
  RVImmI(unsigned fieldIndex)
      : RVImm(fieldIndex, 12, Imm::Repr::Signed,
              std::vector{ImmPart(0, 20, 31)}) {}
};

/// A B-Type RISC-V instruction
struct RVInstrBType : public RVInstruction {
  RVInstrBType(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction(
            RVOpcode(name, RVISA::Opcode::BRANCH, RVOpPartFunct3(funct3)),
            {std::make_shared<RVRegRs1>(isa, 1),
             std::make_shared<RVRegRs2>(isa, 2), std::make_shared<ImmB>()}) {}

  /// A RISC-V signed immediate field with an input width of 13 bits.
  /// Used in B-Type instructions.
  ///
  /// It is defined as:
  ///  - Imm[31:12] = Inst[31]
  ///  - Imm[11]    = Inst[7]
  ///  - Imm[10:5]  = Inst[30:25]
  ///  - Imm[4:1]   = Inst[11:8]
  ///  - Imm[0]     = 0
  struct ImmB : public RVImm {
    ImmB()
        : RVImm(3, 13, Imm::Repr::Signed,
                std::vector{ImmPart(12, 31, 31), ImmPart(11, 7, 7),
                            ImmPart(5, 25, 30), ImmPart(1, 8, 11)},
                Imm::SymbolType::Relative) {}
  };
};

/// A base I-Type RISC-V instruction
struct RVInstrITypeBase : public RVInstruction {
  RVInstrITypeBase(RVISA::Opcode opcode, const Token &name, unsigned funct3,
                   const ISAInfoBase *isa)
      : RVInstruction(RVOpcode(name, opcode, RVOpPartFunct3(funct3)),
                      {std::make_shared<RVRegRd>(isa, 1),
                       std::make_shared<RVRegRs1>(isa, 2),
                       std::make_shared<RVImmI>(3)}) {}
};

/// A I-Type RISC-V instruction
struct RVInstrIType : public RVInstrITypeBase {
  RVInstrIType(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstrITypeBase(RVISA::OPIMM, name, funct3, isa) {}
};

/// A I32-Type RISC-V instruction
struct RVInstrI32Type : public RVInstrITypeBase {
  RVInstrI32Type(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstrITypeBase(RVISA::OPIMM32, name, funct3, isa) {}
};

/// An L-Type RISC-V instruction
struct RVInstrLType : public RVInstruction {
  RVInstrLType(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction(
            RVOpcode(name, RVISA::Opcode::LOAD, RVOpPartFunct3(funct3)),
            {std::make_shared<RVRegRd>(isa, 1),
             std::make_shared<RVRegRs1>(isa, 3), std::make_shared<RVImmI>(2)}) {
  }
};

/// An IShift32-Type RISC-V instruction
struct RVInstrIShift32Type : public RVInstruction {
  RVInstrIShift32Type(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                      unsigned funct7, const ISAInfoBase *isa)
      : RVInstruction(RVOpcode(name, opcode, RVOpPartFunct3(funct3),
                               RVOpPartFunct7(funct7)),
                      {std::make_shared<RVRegRd>(isa, 1),
                       std::make_shared<RVRegRs1>(isa, 2),
                       std::make_shared<ImmIShift32>()}) {}

  /// A RISC-V unsigned immediate field with an input width of 5 bits.
  /// Used in IShift32-Type instructions.
  ///
  /// It is defined as:
  ///  - Imm[4:0] = Inst[24:20]
  struct ImmIShift32 : public RVImm {
    ImmIShift32()
        : RVImm(3, 5, Imm::Repr::Unsigned, std::vector{ImmPart(0, 20, 24)}) {}
  };
};

/// An IShift64-Type RISC-V instruction
struct RVInstrIShift64Type : public RVInstruction {
  RVInstrIShift64Type(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                      unsigned funct6, const ISAInfoBase *isa)
      : RVInstruction(RVOpcode(name, opcode, RVOpPartFunct3(funct3),
                               RVOpPartFunct6(funct6)),
                      {std::make_shared<RVRegRd>(isa, 1),
                       std::make_shared<RVRegRs1>(isa, 2),
                       std::make_shared<ImmIShift64>()}) {}

  /// A RISC-V unsigned immediate field with an input width of 6 bits.
  /// Used in IShift64-Type instructions.
  ///
  /// It is defined as:
  ///  - Imm[5:0] = Inst[25:20]
  struct ImmIShift64 : public RVImm {
    ImmIShift64()
        : RVImm(3, 6, Imm::Repr::Unsigned, std::vector{ImmPart(0, 20, 25)}) {}
  };
};

/// A base R-Type RISC-V instruction
struct RVInstrRTypeBase : public RVInstruction {
  RVInstrRTypeBase(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                   unsigned funct7, const ISAInfoBase *isa)
      : RVInstruction(RVOpcode(name, opcode, RVOpPartFunct3(funct3),
                               RVOpPartFunct7(funct7)),
                      {std::make_shared<RVRegRd>(isa, 1),
                       std::make_shared<RVRegRs1>(isa, 2),
                       std::make_shared<RVRegRs2>(isa, 3)}) {}
};

/// An R-Type RISC-V instruction
struct RVInstrRType : public RVInstrRTypeBase {
  RVInstrRType(const Token &name, unsigned funct3, unsigned funct7,
               const ISAInfoBase *isa)
      : RVInstrRTypeBase(name, RVISA::OP, funct3, funct7, isa) {}
};

/// An R32-Type RISC-V instruction
struct RVInstrR32Type : public RVInstrRTypeBase {
  RVInstrR32Type(const Token &name, unsigned funct3, unsigned funct7,
                 const ISAInfoBase *isa)
      : RVInstrRTypeBase(name, RVISA::OP32, funct3, funct7, isa) {}
};

/// An S-Type RISC-V instruction
struct RVInstrSType : public RVInstruction {
  RVInstrSType(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction(
            RVOpcode(name, RVISA::Opcode::STORE, RVOpPartFunct3(funct3)),
            {std::make_shared<RVRegRs1>(isa, 3), std::make_shared<ImmS>(),
             std::make_shared<RVRegRs2>(isa, 1)}) {}

  /// A RISC-V signed immediate field with an input width of 12 bits.
  /// Used in S-Type instructions.
  ///
  /// It is defined as:
  ///  - Imm[31:11] = Inst[31]
  ///  - Imm[10:5]  = Inst[30:25]
  ///  - Imm[4:0]   = Inst[11:7]
  struct ImmS : public RVImm {
    ImmS()
        : RVImm(2, 12, Imm::Repr::Signed,
                std::vector{ImmPart(5, 25, 31), ImmPart(0, 7, 11)}) {}
  };
};

/// A U-Type RISC-V instruction
struct RVInstrUType : public RVInstruction {
  RVInstrUType(const Token &name, RVISA::Opcode opcode, const ISAInfoBase *isa)
      : RVInstruction(
            RVOpcode(name, opcode),
            {std::make_shared<RVRegRd>(isa, 1), std::make_shared<ImmU>()}) {}

  /// A RISC-V immediate field with an input width of 32 bits.
  /// Used in U-Type instructions.
  ///
  /// It is defined as:
  ///  - Imm[31:12] = Inst[31:12]
  ///  - Imm[11:0]  = 0
  struct ImmU : public RVImm {
    ImmU() : RVImm(2, 32, Imm::Repr::Hex, std::vector{ImmPart(0, 12, 31)}) {}
  };
};

/// A J-Type RISC-V instruction
struct RVInstrJType : public RVInstruction {
  RVInstrJType(const Token &name, RVISA::Opcode opcode, const ISAInfoBase *isa)
      : RVInstruction(
            RVOpcode(name, opcode),
            {std::make_shared<RVRegRd>(isa, 1), std::make_shared<ImmJ>()}) {}

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
  struct ImmJ : public RVImm {
    ImmJ()
        : RVImm(2, 21, Imm::Repr::Signed,
                std::vector{ImmPart(20, 31, 31), ImmPart(12, 12, 19),
                            ImmPart(11, 20, 20), ImmPart(1, 21, 30)},
                Imm::SymbolType::Relative) {}
  };
};

/// A JALR-Type RISC-V instruction
struct RVInstrJALRType : public RVInstruction {
  RVInstrJALRType(const Token &name, const ISAInfoBase *isa)
      : RVInstruction(
            RVOpcode(name, RVISA::Opcode::JALR, RVOpPartFunct3(0b000)),
            {std::make_shared<RVRegRd>(isa, 1),
             std::make_shared<RVRegRs1>(isa, 2), std::make_shared<RVImmI>(3)}) {
  }
};

/// A Load-Type RISC-V pseudo-instruction
struct RVPseudoInstrLoad : public PseudoInstruction {
  RVPseudoInstrLoad(const Token &name)
      : PseudoInstruction(
            name, {PseudoInstruction::reg(), PseudoInstruction::imm()},
            [=](const PseudoInstruction &, const TokenizedSrcLine &line,
                const SymbolMap &) {
              LineTokensVec v;
              v.push_back(LineTokens()
                          << Token("auipc") << line.tokens.at(1)
                          << Token(line.tokens.at(2), "%pcrel_hi"));
              v.push_back(LineTokens()
                          << name << line.tokens.at(1)
                          << Token(QString("(%1 + 4)").arg(line.tokens.at(2)),
                                   "%pcrel_lo")
                          << line.tokens.at(1));
              return v;
            }) {}
};

/// A Store-Type RISC-V pseudo-instruction
///
/// The sw is a pseudo-op if a symbol is given as the immediate token. Thus, if
/// we detect that a number has been provided, then abort the pseudo-op
/// handling.
struct RVPseudoInstrStore : public PseudoInstruction {
  RVPseudoInstrStore(const Token &name)
      : PseudoInstruction(
            name,
            {PseudoInstruction::reg(), PseudoInstruction::imm(),
             PseudoInstruction::reg()},
            [=](const PseudoInstruction &, const TokenizedSrcLine &line,
                const SymbolMap &) {
              bool canConvert;
              getImmediate(line.tokens.at(2), canConvert);
              if (canConvert) {
                return Result<std::vector<LineTokens>>(
                    Error(0, "Unused; will fallback to non-pseudo op sw"));
              }
              LineTokensVec v;
              v.push_back(LineTokens()
                          << Token("auipc") << line.tokens.at(3)
                          << Token(line.tokens.at(2), "%pcrel_hi"));
              v.push_back(LineTokens()
                          << name << line.tokens.at(1)
                          << Token(QString("(%1 + 4)").arg(line.tokens.at(2)),
                                   "%pcrel_lo")
                          << line.tokens.at(3));
              return Result<std::vector<LineTokens>>(v);
            }) {}
};

// The following macros assumes that ASSEMBLER_TYPES(..., ...) has been defined
// for the given assembler.

#define RegTok PseudoInstruction::reg()
#define ImmTok PseudoInstruction::imm()
#define Create_PseudoInstruction
#define _PseudoExpandFuncSyms(line, symbols)                                   \
  [=](const PseudoInstruction &, const TokenizedSrcLine &line,                 \
      const SymbolMap &symbols)

#define _PseudoExpandFunc(line)                                                \
  [](const PseudoInstruction &, const TokenizedSrcLine &line, const SymbolMap &)

} // namespace Assembler
} // namespace Ripes
