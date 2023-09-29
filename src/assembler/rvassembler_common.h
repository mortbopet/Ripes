#pragma once

#include "../isa/rvisainfo_common.h"
#include "assembler.h"

namespace Ripes {
namespace Assembler {

/// A base class for all RISC-V opcode parts
class RVOpPart : public OpPart {
public:
  RVOpPart(unsigned value, unsigned start, unsigned stop)
      : OpPart(value, start, stop) {}
};

/// All RISC-V opcodes are defined as a 7-bit field in bits 0-7 of the
/// instruction
class RVOpPartOpcode : public RVOpPart {
public:
  RVOpPartOpcode(RVISA::Opcode opcode) : RVOpPart(opcode, 0, 6) {}
};

/// All RISC-V instruction quadrants are defined as a 2-bit field in bits 0-1 of
/// the instruction
class RVOpPartQuadrant : public RVOpPart {
public:
  RVOpPartQuadrant(RVISA::Quadrant quadrant) : RVOpPart(quadrant, 0, 1) {}
};

/// All RISC-V Funct3 opcode parts are defined as a 3-bit field in bits 12-14 of
/// the instruction
class RVOpPartFunct3 : public RVOpPart {
public:
  RVOpPartFunct3(unsigned funct3) : RVOpPart(funct3, 12, 14) {}
};

/// All RISC-V Funct6 opcode parts are defined as a 6-bit field in bits 26-31 of
/// the instruction
class RVOpPartFunct6 : public RVOpPart {
public:
  RVOpPartFunct6(unsigned funct6) : RVOpPart(funct6, 26, 31) {}
};

/// All RISC-V Funct7 opcode parts are defined as a 7-bit field in bits 25-31 of
/// the instruction
class RVOpPartFunct7 : public RVOpPart {
public:
  RVOpPartFunct7(unsigned funct7) : RVOpPart(funct7, 25, 31) {}
};

/// A RISC-V opcode defines the encoding of specific instructions
template <typename Reg_T>
class RVOpcode : public Opcode<Reg_T> {
public:
  /// Construct opcode from parts
  RVOpcode(const Token &name, const std::vector<OpPart> &opParts)
      : Opcode<Reg_T>(name, opParts) {}

  /// A RISC-V opcode with no parts
  RVOpcode(const Token &name, RVISA::Opcode opcode)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode)}) {}

  /// A RISC-V opcode with a Funct3 part
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), funct3}) {}

  /// A RISC-V opcode with Funct3 and Funct6 parts
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3,
           RVOpPartFunct6 funct6)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), funct3, funct6}) {}

  /// A RISC-V opcode with Funct3 and Funct7 parts
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3,
           RVOpPartFunct7 funct7)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), funct3, funct7}) {}
};

/// A base class for all RISC-V instructions
template <typename Reg_T>
class RVInstructionBase : public Instruction<Reg_T> {
public:
  RVInstructionBase(const RVOpcode<Reg_T> &opcode,
                    const std::vector<std::shared_ptr<Field<Reg_T>>> &fields)
      : Instruction<Reg_T>(opcode, fields) {}
};

/// RISC-V instruction class (for compressed instructions, see class
/// RVCInstruction)
template <typename Reg_T>
class RVInstruction : public RVInstructionBase<Reg_T> {
public:
  RVInstruction(const RVOpcode<Reg_T> &opcode,
                const std::vector<std::shared_ptr<Field<Reg_T>>> &fields)
      : RVInstructionBase<Reg_T>(opcode, fields) {}
};

/// A base class for all RISC-V register types
template <typename Reg_T>
class RVReg : public Reg<Reg_T> {
public:
  RVReg(const ISAInfoBase *isa, unsigned tokenIndex, unsigned start,
        unsigned stop, const QString &regsd)
      : Reg<Reg_T>(isa, tokenIndex, start, stop, regsd) {}
};

/// The RISC-V Rs1 field contains a source register index.
/// It is defined as a 5-bit field in bits 15-19 of the instruction
template <typename Reg_T>
class RVRegRs1 : public RVReg<Reg_T> {
public:
  RVRegRs1(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg<Reg_T>(isa, fieldIndex, 15, 19, "rs1") {}
};

/// The RISC-V Rs2 field contains a source register index.
/// It is defined as a 5-bit field in bits 20-24 of the instruction
template <typename Reg_T>
class RVRegRs2 : public RVReg<Reg_T> {
public:
  RVRegRs2(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg<Reg_T>(isa, fieldIndex, 20, 24, "rs2") {}
};

/// The RISC-V Rd field contains a destination register index.
/// It is defined as a 5-bit field in bits 7-11 of the instruction
template <typename Reg_T>
class RVRegRd : public RVReg<Reg_T> {
public:
  RVRegRd(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg<Reg_T>(isa, fieldIndex, 7, 11, "rd") {}
};

/// A base class for all RISC-V immediate types
template <typename Reg_T>
class RVImm : public Imm<Reg_T> {
public:
  RVImm(
      unsigned tokenIndex, unsigned width, typename Imm<Reg_T>::Repr repr,
      const std::vector<ImmPart> &parts,
      typename Imm<Reg_T>::SymbolType symbolType = Imm<Reg_T>::SymbolType::None)
      : Imm<Reg_T>(tokenIndex, width, repr, parts, symbolType) {}
};

/// A RISC-V unsigned immediate field with an input width of 5 bits.
/// Used in IShift32-Type instructions.
///
/// It is defined as:
///  - Imm[4:0] = Inst[24:20]
template <typename Reg_T>
class RVImmIShift32 : public RVImm<Reg_T> {
public:
  RVImmIShift32()
      : RVImm<Reg_T>(3, 5, Imm<Reg_T>::Repr::Unsigned,
                     std::vector{ImmPart(0, 20, 24)}) {}
};

/// A RISC-V unsigned immediate field with an input width of 6 bits.
/// Used in IShift64-Type instructions.
///
/// It is defined as:
///  - Imm[5:0] = Inst[25:20]
template <typename Reg_T>
class RVImmIShift64 : public RVImm<Reg_T> {
public:
  RVImmIShift64()
      : RVImm<Reg_T>(3, 6, Imm<Reg_T>::Repr::Unsigned,
                     std::vector{ImmPart(0, 20, 25)}) {}
};

/// A RISC-V signed immediate field with an input width of 12 bits.
/// Used in L-Type and I-Type instructions.
///
/// It is defined as:
///  - Imm[31:11] = Inst[31]
///  - Imm[10:0]  = Inst[30:20]
template <typename Reg_T>
class RVImmI : public RVImm<Reg_T> {
public:
  RVImmI(unsigned fieldIndex)
      : RVImm<Reg_T>(fieldIndex, 12, Imm<Reg_T>::Repr::Signed,
                     std::vector{ImmPart(0, 20, 31)}) {}
};

/// A RISC-V signed immediate field with an input width of 12 bits.
/// Used in S-Type instructions.
///
/// It is defined as:
///  - Imm[31:11] = Inst[31]
///  - Imm[10:5]  = Inst[30:25]
///  - Imm[4:0]   = Inst[11:7]
template <typename Reg_T>
class RVImmS : public RVImm<Reg_T> {
public:
  RVImmS()
      : RVImm<Reg_T>(2, 12, Imm<Reg_T>::Repr::Signed,
                     std::vector{ImmPart(5, 25, 31), ImmPart(0, 7, 11)}) {}
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
template <typename Reg_T>
class RVImmB : public RVImm<Reg_T> {
public:
  RVImmB()
      : RVImm<Reg_T>(3, 13, Imm<Reg_T>::Repr::Signed,
                     std::vector{ImmPart(12, 31, 31), ImmPart(11, 7, 7),
                                 ImmPart(5, 25, 30), ImmPart(1, 8, 11)},
                     Imm<Reg_T>::SymbolType::Relative) {}
};

/// A RISC-V immediate field with an input width of 32 bits.
/// Used in U-Type instructions.
///
/// It is defined as:
///  - Imm[31:12] = Inst[31:12]
///  - Imm[11:0]  = 0
template <typename Reg_T>
class RVImmU : public RVImm<Reg_T> {
public:
  RVImmU()
      : RVImm<Reg_T>(2, 32, Imm<Reg_T>::Repr::Hex,
                     std::vector{ImmPart(0, 12, 31)}) {}
};

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
template <typename Reg_T>
class RVImmJ : public RVImm<Reg_T> {
public:
  RVImmJ()
      : RVImm<Reg_T>(2, 21, Imm<Reg_T>::Repr::Signed,
                     std::vector{ImmPart(20, 31, 31), ImmPart(12, 12, 19),
                                 ImmPart(11, 20, 20), ImmPart(1, 21, 30)},
                     Imm<Reg_T>::SymbolType::Relative) {}
};

/// A B-Type RISC-V instruction
template <typename Reg_T>
class RVInstrBType : public RVInstruction<Reg_T> {
public:
  RVInstrBType(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(RVOpcode<Reg_T>(name, RVISA::Opcode::BRANCH,
                                             RVOpPartFunct3(funct3)),
                             {std::make_shared<RVRegRs1<Reg_T>>(isa, 1),
                              std::make_shared<RVRegRs2<Reg_T>>(isa, 2),
                              std::make_shared<RVImmB<Reg_T>>()}) {}
};

/// A base I-Type RISC-V instruction
template <typename Reg_T>
class RVInstrITypeBase : public RVInstruction<Reg_T> {
public:
  RVInstrITypeBase(RVISA::Opcode opcode, const Token &name, unsigned funct3,
                   const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, opcode, RVOpPartFunct3(funct3)),
            {std::make_shared<RVRegRd<Reg_T>>(isa, 1),
             std::make_shared<RVRegRs1<Reg_T>>(isa, 2),
             std::make_shared<RVImmI<Reg_T>>(3)}) {}
};

/// A I-Type RISC-V instruction
template <typename Reg_T>
class RVInstrIType : public RVInstrITypeBase<Reg_T> {
public:
  RVInstrIType(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstrITypeBase<Reg_T>(RVISA::OPIMM, name, funct3, isa) {}
};

/// A I32-Type RISC-V instruction
template <typename Reg_T>
class RVInstrI32Type : public RVInstrITypeBase<Reg_T> {
public:
  RVInstrI32Type(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstrITypeBase<Reg_T>(RVISA::OPIMM32, name, funct3, isa) {}
};

/// An L-Type RISC-V instruction
template <typename Reg_T>
class RVInstrLType : public RVInstruction<Reg_T> {
public:
  RVInstrLType(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, RVISA::Opcode::LOAD, RVOpPartFunct3(funct3)),
            {std::make_shared<RVRegRd<Reg_T>>(isa, 1),
             std::make_shared<RVRegRs1<Reg_T>>(isa, 3),
             std::make_shared<RVImmI<Reg_T>>(2)}) {}
};

/// An IShift32-Type RISC-V instruction
template <typename Reg_T>
class RVInstrIShift32Type : public RVInstruction<Reg_T> {
public:
  RVInstrIShift32Type(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                      unsigned funct7, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(RVOpcode<Reg_T>(name, opcode,
                                             RVOpPartFunct3(funct3),
                                             RVOpPartFunct7(funct7)),
                             {std::make_shared<RVRegRd<Reg_T>>(isa, 1),
                              std::make_shared<RVRegRs1<Reg_T>>(isa, 2),
                              std::make_shared<RVImmIShift32<Reg_T>>()}) {}
};

/// An IShift64-Type RISC-V instruction
template <typename Reg_T>
class RVInstrIShift64Type : public RVInstruction<Reg_T> {
public:
  RVInstrIShift64Type(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                      unsigned funct6, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(RVOpcode<Reg_T>(name, opcode,
                                             RVOpPartFunct3(funct3),
                                             RVOpPartFunct6(funct6)),
                             {std::make_shared<RVRegRd<Reg_T>>(isa, 1),
                              std::make_shared<RVRegRs1<Reg_T>>(isa, 2),
                              std::make_shared<RVImmIShift64<Reg_T>>()}) {}
};

/// A base R-Type RISC-V instruction
template <typename Reg_T>
class RVInstrRTypeBase : public RVInstruction<Reg_T> {
public:
  RVInstrRTypeBase(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                   unsigned funct7, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(RVOpcode<Reg_T>(name, opcode,
                                             RVOpPartFunct3(funct3),
                                             RVOpPartFunct7(funct7)),
                             {std::make_shared<RVRegRd<Reg_T>>(isa, 1),
                              std::make_shared<RVRegRs1<Reg_T>>(isa, 2),
                              std::make_shared<RVRegRs2<Reg_T>>(isa, 3)}) {}
};

/// An R-Type RISC-V instruction
template <typename Reg_T>
class RVInstrRType : public RVInstrRTypeBase<Reg_T> {
public:
  RVInstrRType(const Token &name, unsigned funct3, unsigned funct7,
               const ISAInfoBase *isa)
      : RVInstrRTypeBase<Reg_T>(name, RVISA::OP, funct3, funct7, isa) {}
};

/// An R32-Type RISC-V instruction
template <typename Reg_T>
class RVInstrR32Type : public RVInstrRTypeBase<Reg_T> {
public:
  RVInstrR32Type(const Token &name, unsigned funct3, unsigned funct7,
                 const ISAInfoBase *isa)
      : RVInstrRTypeBase<Reg_T>(name, RVISA::OP32, funct3, funct7, isa) {}
};

/// An S-Type RISC-V instruction
template <typename Reg_T>
class RVInstrSType : public RVInstruction<Reg_T> {
public:
  RVInstrSType(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, RVISA::Opcode::STORE, RVOpPartFunct3(funct3)),
            {std::make_shared<RVRegRs1<Reg_T>>(isa, 3),
             std::make_shared<RVImmS<Reg_T>>(),
             std::make_shared<RVRegRs2<Reg_T>>(isa, 1)}) {}
};

/// A U-Type RISC-V instruction
template <typename Reg_T>
class RVInstrUType : public RVInstruction<Reg_T> {
public:
  RVInstrUType(const Token &name, RVISA::Opcode opcode, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(RVOpcode<Reg_T>(name, opcode),
                             {std::make_shared<RVRegRd<Reg_T>>(isa, 1),
                              std::make_shared<RVImmU<Reg_T>>()}) {}
};

/// A J-Type RISC-V instruction
template <typename Reg_T>
class RVInstrJType : public RVInstruction<Reg_T> {
public:
  RVInstrJType(const Token &name, RVISA::Opcode opcode, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(RVOpcode<Reg_T>(name, opcode),
                             {std::make_shared<RVRegRd<Reg_T>>(isa, 1),
                              std::make_shared<RVImmJ<Reg_T>>()}) {}
};

/// A JALR-Type RISC-V instruction
template <typename Reg_T>
class RVInstrJALRType : public RVInstruction<Reg_T> {
public:
  RVInstrJALRType(const Token &name, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, RVISA::Opcode::JALR, RVOpPartFunct3(0b000)),
            {std::make_shared<RVRegRd<Reg_T>>(isa, 1),
             std::make_shared<RVRegRs1<Reg_T>>(isa, 2),
             std::make_shared<RVImmI<Reg_T>>(3)}) {}
};

/// A Load-Type RISC-V pseudo-instruction
template <typename Reg_T>
class RVPseudoInstrLoad : public PseudoInstruction<Reg_T> {
public:
  RVPseudoInstrLoad(const Token &name)
      : PseudoInstruction<Reg_T>(
            name,
            {PseudoInstruction<Reg_T>::reg(), PseudoInstruction<Reg_T>::imm()},
            [=](const PseudoInstruction<Reg_T> &, const TokenizedSrcLine &line,
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
template <typename Reg_T>
class RVPseudoInstrStore : public PseudoInstruction<Reg_T> {
public:
  RVPseudoInstrStore(const Token &name)
      : PseudoInstruction<Reg_T>(
            name,
            {PseudoInstruction<Reg_T>::reg(), PseudoInstruction<Reg_T>::imm(),
             PseudoInstruction<Reg_T>::reg()},
            [=](const PseudoInstruction<Reg_T> &, const TokenizedSrcLine &line,
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

#define RegTok _PseudoInstruction::reg()
#define ImmTok _PseudoInstruction::imm()
#define Create_PseudoInstruction
#define _PseudoExpandFuncSyms(line, symbols)                                   \
  [=](const _PseudoInstruction &, const TokenizedSrcLine &line,                \
      const SymbolMap &symbols)

#define _PseudoExpandFunc(line)                                                \
  [](const _PseudoInstruction &, const TokenizedSrcLine &line,                 \
     const SymbolMap &)

} // namespace Assembler
} // namespace Ripes
