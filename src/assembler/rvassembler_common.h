#pragma once

#include "../isa/rvisainfo_common.h"
#include "assembler.h"

namespace Ripes {
namespace Assembler {
template <typename Reg_T>
// A base class for all RISC-V instructions
class RVInstructionBase : public Instruction<Reg_T> {
public:
  RVInstructionBase(const Opcode<Reg_T> &opcode,
                    const std::vector<std::shared_ptr<Field<Reg_T>>> &fields)
      : Instruction<Reg_T>(opcode, fields) {}
};

// RISC-V instruction class (for compressed instructions, see class
// RVCInstruction)
template <typename Reg_T>
class RVInstruction : public RVInstructionBase<Reg_T> {
public:
  RVInstruction(const Opcode<Reg_T> &opcode,
                const std::vector<std::shared_ptr<Field<Reg_T>>> &fields)
      : RVInstructionBase<Reg_T>(opcode, fields) {}
};

// A base class for all RISC-V opcode parts
class RVOpPart : public OpPart {
public:
  RVOpPart(unsigned value, unsigned start, unsigned stop)
      : OpPart(value, start, stop) {}
};

// All RISC-V opcodes are defined as the 7 LSBs of the instruction
class RVOpPartOpcode : public RVOpPart {
public:
  RVOpPartOpcode(RVISA::Opcode opcode) : RVOpPart(opcode, 0, 6) {}
};

// All RISC-V instruction quadrants are defined as the 2 LSBs of the instruction
class RVOpPartQuadrant : public RVOpPart {
public:
  RVOpPartQuadrant(RVISA::Quadrant quadrant) : RVOpPart(quadrant, 0, 1) {}
};

// All RISC-V Funct3 opcode parts are defined as bits 12-14 (inclusive) of the
// instruction
class RVOpPartFunct3 : public RVOpPart {
public:
  RVOpPartFunct3(unsigned funct3) : RVOpPart(funct3, 12, 14) {}
};

// All RISC-V Funct6 opcode parts are defined as bits 26-31 (inclusive) of the
// instruction
class RVOpPartFunct6 : public RVOpPart {
public:
  RVOpPartFunct6(unsigned funct6) : RVOpPart(funct6, 26, 31) {}
};

// All RISC-V Funct7 opcode parts are defined as bits 25-31 (inclusive) of the
// instruction
class RVOpPartFunct7 : public RVOpPart {
public:
  RVOpPartFunct7(unsigned funct7) : RVOpPart(funct7, 25, 31) {}
};

// A RISC-V opcode defines the encoding of specific instructions
template <typename Reg_T>
class RVOpcode : public Opcode<Reg_T> {
public:
  // Construct opcode from parts
  RVOpcode(const Token &name, const std::vector<OpPart> &opParts)
      : Opcode<Reg_T>(name, opParts) {}

  // A RISC-V opcode with no parts
  RVOpcode(const Token &name, RVISA::Opcode opcode)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode)}) {}

  // A RISC-V opcode with a Funct3 part
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), funct3}) {}

  // A RISC-V opcode with Funct3 and Funct6 parts
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3,
           RVOpPartFunct6 funct6)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), funct3, funct6}) {}

  // A RISC-V opcode with Funct3 and Funct7 parts
  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3,
           RVOpPartFunct7 funct7)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), funct3, funct7}) {}
};

// A base class for all RISC-V register types
template <typename Reg_T>
class RVReg : public Reg<Reg_T> {
public:
  RVReg(const ISAInfoBase *isa, unsigned tokenIndex, unsigned start,
        unsigned stop, const QString &regsd)
      : Reg<Reg_T>(isa, tokenIndex, start, stop, regsd) {}
};

// The RISC-V Rs1 field contains a source register index.
// It is defined as bits 15-19 (inclusive)
template <typename Reg_T>
class RVRegRs1 : public RVReg<Reg_T> {
public:
  RVRegRs1(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg<Reg_T>(isa, fieldIndex, 15, 19, "rs1") {}
};

// The RISC-V Rs2 field contains a source register index.
// It is defined as bits 20-24 (inclusive)
template <typename Reg_T>
class RVRegRs2 : public RVReg<Reg_T> {
public:
  RVRegRs2(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg<Reg_T>(isa, fieldIndex, 20, 24, "rs2") {}
};

// The RISC-V Rd field contains a destination register index.
// It is defined as bits 7-11 (inclusive)
template <typename Reg_T>
class RVRegRd : public RVReg<Reg_T> {
public:
  RVRegRd(const ISAInfoBase *isa, unsigned fieldIndex)
      : RVReg<Reg_T>(isa, fieldIndex, 7, 11, "rd") {}
};

// A base class for all RISC-V immediate types
template <typename Reg_T>
class RVImm : public Imm<Reg_T> {
public:
  RVImm(
      unsigned tokenIndex, unsigned width, typename Imm<Reg_T>::Repr repr,
      const std::vector<ImmPart> &parts,
      typename Imm<Reg_T>::SymbolType symbolType = Imm<Reg_T>::SymbolType::None)
      : Imm<Reg_T>(tokenIndex, width, repr, parts, symbolType) {}
};

// A RISC-V unsigned immediate field with an input width of 5 bits.
// Used in I-Shift32-Type instructions.
//
// It is defined as:
//  - Imm[4:0] = Inst[24:20]
template <typename Reg_T>
class RVImmIShift32Type : public RVImm<Reg_T> {
public:
  RVImmIShift32Type()
      : RVImm<Reg_T>(3, 5, Imm<Reg_T>::Repr::Unsigned,
                     std::vector{ImmPart(0, 20, 24)}) {}
};

// A RISC-V unsigned immediate field with an input width of 6 bits.
// Used in I-Shift64-Type instructions.
//
// It is defined as:
//  - Imm[5:0] = Inst[25:20]
template <typename Reg_T>
class RVImmIShift64Type : public RVImm<Reg_T> {
public:
  RVImmIShift64Type()
      : RVImm<Reg_T>(3, 6, Imm<Reg_T>::Repr::Unsigned,
                     std::vector{ImmPart(0, 20, 25)}) {}
};

// A RISC-V signed immediate field with an input width of 12 bits.
// Used in L-Type and I-Type instructions.
//
// It is defined as:
//  - Imm[31:11] = Inst[31]
//  - Imm[10:0]  = Inst[30:20]
template <typename Reg_T>
class RVImmIType : public RVImm<Reg_T> {
public:
  RVImmIType(unsigned fieldIndex)
      : RVImm<Reg_T>(fieldIndex, 12, Imm<Reg_T>::Repr::Signed,
                     std::vector{ImmPart(0, 20, 31)}) {}
};

// A RISC-V signed immediate field with an input width of 12 bits.
// Used in S-Type instructions.
//
// It is defined as:
//  - Imm[31:11] = Inst[31]
//  - Imm[10:5]  = Inst[30:25]
//  - Imm[4:0]   = Inst[11:7]
template <typename Reg_T>
class RVImmSType : public RVImm<Reg_T> {
public:
  RVImmSType()
      : RVImm<Reg_T>(2, 12, Imm<Reg_T>::Repr::Signed,
                     std::vector{ImmPart(5, 25, 31), ImmPart(0, 7, 11)}) {}
};

// A RISC-V signed immediate field with an input width of 13 bits.
// Used in B-Type instructions.
//
// It is defined as:
//  - Imm[31:12] = Inst[31]
//  - Imm[11]    = Inst[7]
//  - Imm[10:5]  = Inst[30:25]
//  - Imm[4:1]   = Inst[11:8]
//  - Imm[0]     = 0
template <typename Reg_T>
class RVImmBType : public RVImm<Reg_T> {
public:
  RVImmBType()
      : RVImm<Reg_T>(3, 13, Imm<Reg_T>::Repr::Signed,
                     std::vector{ImmPart(12, 31, 31), ImmPart(11, 7, 7),
                                 ImmPart(5, 25, 30), ImmPart(1, 8, 11)},
                     Imm<Reg_T>::SymbolType::Relative) {}
};

// A RISC-V signed immediate field with an input width of 20 bits.
// Used in U-Type instructions.
//
// It is defined as:
//  - Imm[31:12] = Inst[31:12]
//  - Imm[11:0]  = 0
template <typename Reg_T>
class RVImmUType : public RVImm<Reg_T> {
public:
  RVImmUType()
      : RVImm<Reg_T>(2, 32, Imm<Reg_T>::Repr::Hex,
                     std::vector{ImmPart(0, 12, 31)}) {}
};

// A RISC-V signed immediate field with an input width of 21 bits.
// Used in J-Type instructions.
//
// It is defined as:
//  - Imm[31:20] = Inst[31]
//  - Imm[19:12] = Inst[19:12]
//  - Imm[11]    = Inst[20]
//  - Imm[10:5]  = Inst[30:25]
//  - Imm[4:1]   = Inst[24:21]
//  - Imm[0]     = 0
template <typename Reg_T>
class RVImmJType : public RVImm<Reg_T> {
public:
  RVImmJType()
      : RVImm<Reg_T>(2, 21, Imm<Reg_T>::Repr::Signed,
                     std::vector{ImmPart(20, 31, 31), ImmPart(12, 12, 19),
                                 ImmPart(11, 20, 20), ImmPart(1, 21, 30)},
                     Imm<Reg_T>::SymbolType::Relative) {}
};

// A B-Type RISC-V instruction
template <typename Reg_T>
class BTypeInstr : public RVInstruction<Reg_T> {
public:
  BTypeInstr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, RVISA::Opcode::BRANCH,
                            RVOpPartFunct3(funct3)),
            {std::make_shared<Reg<Reg_T>>(RVRegRs1<Reg_T>(isa, 1)),
             std::make_shared<Reg<Reg_T>>(RVRegRs2<Reg_T>(isa, 2)),
             std::make_shared<Imm<Reg_T>>(RVImmBType<Reg_T>())}) {}
};

template <typename Reg_T>
class ITypeInstrCommon : public RVInstruction<Reg_T> {
public:
  ITypeInstrCommon(RVISA::Opcode opcode, const Token &name, unsigned funct3,
                   const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, opcode, RVOpPartFunct3(funct3)),
            {std::make_shared<Reg<Reg_T>>(RVRegRd<Reg_T>(isa, 1)),
             std::make_shared<Reg<Reg_T>>(RVRegRs1<Reg_T>(isa, 2)),
             std::make_shared<Imm<Reg_T>>(RVImmIType<Reg_T>(3))}) {}
};

template <typename Reg_T>
class ITypeInstr : public ITypeInstrCommon<Reg_T> {
public:
  ITypeInstr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : ITypeInstrCommon<Reg_T>(RVISA::OPIMM, name, funct3, isa) {}
};

template <typename Reg_T>
class IType32Instr : public ITypeInstrCommon<Reg_T> {
public:
  IType32Instr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : ITypeInstrCommon<Reg_T>(RVISA::OPIMM32, name, funct3, isa) {}
};

template <typename Reg_T>
class LTypeInstr : public RVInstruction<Reg_T> {
public:
  LTypeInstr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, RVISA::Opcode::LOAD, RVOpPartFunct3(funct3)),
            {std::make_shared<Reg<Reg_T>>(RVRegRd<Reg_T>(isa, 1)),
             std::make_shared<Reg<Reg_T>>(RVRegRs1<Reg_T>(isa, 3)),
             std::make_shared<Imm<Reg_T>>(RVImmIType<Reg_T>(2))}) {}
};

template <typename Reg_T>
class IShiftType32Instr : public RVInstruction<Reg_T> {
public:
  IShiftType32Instr(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                    unsigned funct7, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, opcode, RVOpPartFunct3(funct3),
                            RVOpPartFunct7(funct7)),
            {std::make_shared<Reg<Reg_T>>(RVRegRd<Reg_T>(isa, 1)),
             std::make_shared<Reg<Reg_T>>(RVRegRs1<Reg_T>(isa, 2)),
             std::make_shared<Imm<Reg_T>>(RVImmIShift32Type<Reg_T>())}) {}
};

template <typename Reg_T>
class IShiftType64Instr : public RVInstruction<Reg_T> {
public:
  IShiftType64Instr(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                    unsigned funct6, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, opcode, RVOpPartFunct3(funct3),
                            RVOpPartFunct6(funct6)),
            {std::make_shared<Reg<Reg_T>>(RVRegRd<Reg_T>(isa, 1)),
             std::make_shared<Reg<Reg_T>>(RVRegRs1<Reg_T>(isa, 2)),
             std::make_shared<Imm<Reg_T>>(RVImmIShift64Type<Reg_T>())}) {}
};

template <typename Reg_T>
class RTypeInstrCommon : public RVInstruction<Reg_T> {
public:
  RTypeInstrCommon(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                   unsigned funct7, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, opcode, RVOpPartFunct3(funct3),
                            RVOpPartFunct7(funct7)),
            {std::make_shared<Reg<Reg_T>>(RVRegRd<Reg_T>(isa, 1)),
             std::make_shared<Reg<Reg_T>>(RVRegRs1<Reg_T>(isa, 2)),
             std::make_shared<Reg<Reg_T>>(RVRegRs2<Reg_T>(isa, 3))}) {}
};

template <typename Reg_T>
class RTypeInstr : public RTypeInstrCommon<Reg_T> {
public:
  RTypeInstr(const Token &name, unsigned funct3, unsigned funct7,
             const ISAInfoBase *isa)
      : RTypeInstrCommon<Reg_T>(name, RVISA::OP, funct3, funct7, isa) {}
};

template <typename Reg_T>
class RType32Instr : public RTypeInstrCommon<Reg_T> {
public:
  RType32Instr(const Token &name, unsigned funct3, unsigned funct7,
               const ISAInfoBase *isa)
      : RTypeInstrCommon<Reg_T>(name, RVISA::OP32, funct3, funct7, isa) {}
};

template <typename Reg_T>
class STypeInstr : public RVInstruction<Reg_T> {
public:
  STypeInstr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, RVISA::Opcode::STORE, RVOpPartFunct3(funct3)),
            {std::make_shared<Reg<Reg_T>>(RVRegRs1<Reg_T>(isa, 3)),
             std::make_shared<Imm<Reg_T>>(RVImmSType<Reg_T>()),
             std::make_shared<Reg<Reg_T>>(RVRegRs2<Reg_T>(isa, 1))}) {}
};

template <typename Reg_T>
class UTypeInstr : public RVInstruction<Reg_T> {
public:
  UTypeInstr(const Token &name, RVISA::Opcode opcode, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, opcode),
            {std::make_shared<Reg<Reg_T>>(RVRegRd<Reg_T>(isa, 1)),
             std::make_shared<Imm<Reg_T>>(RVImmUType<Reg_T>())}) {}
};

template <typename Reg_T>
class JTypeInstr : public RVInstruction<Reg_T> {
public:
  JTypeInstr(const Token &name, RVISA::Opcode opcode, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, opcode),
            {std::make_shared<Reg<Reg_T>>(RVRegRd<Reg_T>(isa, 1)),
             std::make_shared<Imm<Reg_T>>(RVImmJType<Reg_T>())}) {}
};

template <typename Reg_T>
class JALRTypeInstr : public RVInstruction<Reg_T> {
public:
  JALRTypeInstr(const Token &name, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            RVOpcode<Reg_T>(name, RVISA::Opcode::JALR, RVOpPartFunct3(0b000)),
            {std::make_shared<Reg<Reg_T>>(RVRegRd<Reg_T>(isa, 1)),
             std::make_shared<Reg<Reg_T>>(RVRegRs1<Reg_T>(isa, 2)),
             std::make_shared<Imm<Reg_T>>(RVImmIType<Reg_T>(3))}) {}
};

template <typename Reg_T>
class PseudoLoadInstr : public PseudoInstruction<Reg_T> {
public:
  PseudoLoadInstr(const Token &name)
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

// The sw is a pseudo-op if a symbol is given as the immediate token. Thus, if
// we detect that a number has been provided, then abort the pseudo-op handling.
template <typename Reg_T>
class PseudoStoreInstr : public PseudoInstruction<Reg_T> {
public:
  PseudoStoreInstr(const Token &name)
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
