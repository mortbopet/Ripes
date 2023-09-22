#pragma once

#include "../isa/rvisainfo_common.h"
#include "assembler.h"

namespace Ripes {
namespace Assembler {

// A base class for RISC-V instructions
template <typename Reg_T>
class RVInstruction : public Instruction<Reg_T> {
public:
  RVInstruction(const Opcode<Reg_T> &opcode,
                const std::vector<std::shared_ptr<Field<Reg_T>>> &fields)
      : Instruction<Reg_T>(opcode, fields) {}
};

class RVOpPartOpcode : public OpPart {
public:
  RVOpPartOpcode(RVISA::Opcode opcode)
      : OpPart(opcode, 0, 6) {}
};

class RVOpPartFunct3 : public OpPart {
public:
  RVOpPartFunct3(unsigned funct3)
      : OpPart(funct3, 12, 14) {}
};

class RVOpPartFunct6 : public OpPart {
public:
  RVOpPartFunct6(unsigned funct6)
      : OpPart(funct6, 26, 31) {}
};

class RVOpPartFunct7 : public OpPart {
public:
  RVOpPartFunct7(unsigned funct7)
      : OpPart(funct7, 25, 31) {}
};

template <typename Reg_T>
class RVOpcode : public Opcode<Reg_T> {
public:
  RVOpcode(const Token &name, RVISA::Opcode opcode)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode)}) {}

  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), funct3}) {}

  RVOpcode(const Token &name, RVISA::Opcode opcode, RVOpPartFunct3 funct3,
           RVOpPartFunct7 funct7)
      : Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), funct3, funct7}) {}
};

template <typename Reg_T>
class RVRegRs1 : public Reg<Reg_T> {
public:
  RVRegRs1(const ISAInfoBase *isa, unsigned fieldIndex)
      : Reg<Reg_T>(isa, fieldIndex, 15, 19, "rs1") {}
};

template <typename Reg_T>
class RVRegRs2 : public Reg<Reg_T> {
public:
  RVRegRs2(const ISAInfoBase *isa, unsigned fieldIndex)
      : Reg<Reg_T>(isa, fieldIndex, 20, 24, "rs2") {}
};

template <typename Reg_T>
class RVRegRd : public Reg<Reg_T> {
public:
  RVRegRd(const ISAInfoBase *isa, unsigned fieldIndex)
      : Reg<Reg_T>(isa, fieldIndex, 7, 11, "rd") {}
};

template <typename Reg_T, class RVReg_T>
std::shared_ptr<Reg<Reg_T>> makeRvReg(const ISAInfoBase *isa, unsigned fieldIndex) {
  return std::make_shared<Reg<Reg_T>>(RVReg_T(isa, fieldIndex));
}

template <typename Reg_T>
class RVImm5 : public Imm<Reg_T> {
public:
  RVImm5(unsigned fieldIndex)
      : Imm<Reg_T>(fieldIndex, 5, Imm<Reg_T>::Repr::Unsigned,
                   std::vector{ImmPart(0, 20, 24)}) {}
};

template <typename Reg_T>
class RVImm12 : public Imm<Reg_T> {
public:
  RVImm12(unsigned fieldIndex)
      : Imm<Reg_T>(fieldIndex, 12, Imm<Reg_T>::Repr::Signed,
                   std::vector{ImmPart(0, 20, 31)}) {}
};

template <typename Reg_T>
class RVImm13 : public Imm<Reg_T> {
public:
  RVImm13(unsigned fieldIndex)
      : Imm<Reg_T>(fieldIndex, 13, Imm<Reg_T>::Repr::Signed,
                   std::vector{ImmPart(12, 31, 31), ImmPart(11, 7, 7),
                               ImmPart(5, 25, 30), ImmPart(1, 8, 11)},
                   Imm<Reg_T>::SymbolType::Relative) {}
};

// A B-Type RISC-V instruction
template <typename Reg_T>
class BTypeInstr : public RVInstruction<Reg_T> {
public:
  BTypeInstr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            Opcode<Reg_T>(name, {RVOpPartOpcode(RVISA::Opcode::BRANCH),
                          RVOpPartFunct3(funct3)}),
            {makeRvReg<Reg_T, RVRegRs1<Reg_T>>(isa, 1),
             makeRvReg<Reg_T, RVRegRs2<Reg_T>>(isa, 2),
             std::make_shared<Imm<Reg_T>>(RVImm13<Reg_T>(3))
            }) {}
};

template <typename Reg_T>
std::shared_ptr<Instruction<Reg_T>> defineBType(const QString &name, unsigned funct3,
                                                       const ISAInfoBase *isa) {
  return std::shared_ptr<Instruction<Reg_T>>(
      new BTypeInstr<Reg_T>(Token(name), funct3, isa));
}

template <typename Reg_T>
class ITypeInstrCommon : public RVInstruction<Reg_T> {
public:
  ITypeInstrCommon(RVISA::Opcode opcode, const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), RVOpPartFunct3(funct3)}),
            {makeRvReg<Reg_T, RVRegRd<Reg_T>>(isa, 1),
             makeRvReg<Reg_T, RVRegRs1<Reg_T>>(isa, 2),
             std::make_shared<Imm<Reg_T>>(RVImm12<Reg_T>(3))
            }) {}
};

template <typename Reg_T>
class ITypeInstr : public ITypeInstrCommon<Reg_T> {
public:
  ITypeInstr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : ITypeInstrCommon<Reg_T>(RVISA::OPIMM, name, funct3, isa) {}
};

template <typename Reg_T>
std::shared_ptr<Instruction<Reg_T>> defineIType(const QString &name, unsigned funct3,
                                                const ISAInfoBase *isa) {
  return std::shared_ptr<Instruction<Reg_T>>(
      new ITypeInstr<Reg_T>(Token(name), funct3, isa));
}

template <typename Reg_T>
class IType32Instr : public ITypeInstrCommon<Reg_T> {
public:
  IType32Instr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : ITypeInstrCommon<Reg_T>(RVISA::OPIMM32, name, funct3, isa) {}
};

template <typename Reg_T>
std::shared_ptr<Instruction<Reg_T>> defineIType32(const QString &name, unsigned funct3,
                                                const ISAInfoBase *isa) {
  return std::shared_ptr<Instruction<Reg_T>>(
      new IType32Instr<Reg_T>(Token(name), funct3, isa));
}

template <typename Reg_T>
class LTypeInstr : public RVInstruction<Reg_T> {
public:
  LTypeInstr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            Opcode<Reg_T>(name, {RVOpPartOpcode(RVISA::Opcode::LOAD),
                                 RVOpPartFunct3(funct3)}),
            {makeRvReg<Reg_T, RVRegRd<Reg_T>>(isa, 1),
             makeRvReg<Reg_T, RVRegRs1<Reg_T>>(isa, 3),
             std::make_shared<Imm<Reg_T>>(RVImm12<Reg_T>(2))
            }) {}
};

template <typename Reg_T>
std::shared_ptr<Instruction<Reg_T>> defineLType(const QString &name, unsigned funct3,
                                                  const ISAInfoBase *isa) {
  return std::shared_ptr<Instruction<Reg_T>>(
      new LTypeInstr<Reg_T>(Token(name), funct3, isa));
}

template <typename Reg_T>
class IShiftType32Instr : public RVInstruction<Reg_T> {
public:
  IShiftType32Instr(const Token &name, RVISA::Opcode opcode, unsigned funct3,
                    unsigned funct7, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            Opcode<Reg_T>(name, {RVOpPartOpcode(opcode), RVOpPartFunct3(funct3),
                                 RVOpPartFunct7(funct7)}),
            {makeRvReg<Reg_T, RVRegRd<Reg_T>>(isa, 1),
             makeRvReg<Reg_T, RVRegRs1<Reg_T>>(isa, 2),
             std::make_shared<Imm<Reg_T>>(RVImm5<Reg_T>(3))}) {}
};

template <typename Reg_T>
std::shared_ptr<Instruction<Reg_T>> defineIShiftType32(const QString &name,
    RVISA::Opcode opcode, unsigned funct3, unsigned funct7, const ISAInfoBase *isa) {
  return std::shared_ptr<Instruction<Reg_T>>(
      new IShiftType32Instr<Reg_T>(Token(name), opcode, funct3, funct7, isa));
}

// The following macros assumes that ASSEMBLER_TYPES(..., ...) has been defined
// for the given assembler.

#define IShiftType64(name, opcode, funct3, funct6)                             \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      _Opcode(name, {OpPart(opcode, 0, 6), OpPart(funct3, 12, 14),             \
                     OpPart(funct6, 26, 31)}),                                 \
      {std::make_shared<_Reg>(isa, 1, 7, 11, "rd"),                            \
       std::make_shared<_Reg>(isa, 2, 15, 19, "rs1"),                          \
       std::make_shared<_Imm>(3, 6, _Imm::Repr::Unsigned,                      \
                              std::vector{ImmPart(0, 20, 25)})}))

#define RTypeCommon(name, opcode, funct3, funct7)                              \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      _Opcode(name, {OpPart(opcode, 0, 6), OpPart(funct3, 12, 14),             \
                     OpPart(funct7, 25, 31)}),                                 \
      {std::make_shared<_Reg>(isa, 1, 7, 11, "rd"),                            \
       std::make_shared<_Reg>(isa, 2, 15, 19, "rs1"),                          \
       std::make_shared<_Reg>(isa, 3, 20, 24, "rs2")}))

#define RType(name, funct3, funct7) RTypeCommon(name, RVISA::OP, funct3, funct7)
#define RType32(name, funct3, funct7)                                          \
  RTypeCommon(name, RVISA::OP32, funct3, funct7)

#define SType(name, funct3)                                                    \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      _Opcode(name,                                                            \
              {OpPart(RVISA::Opcode::STORE, 0, 6), OpPart(funct3, 12, 14)}),   \
      {std::make_shared<_Reg>(isa, 3, 15, 19, "rs1"),                          \
       std::make_shared<_Imm>(                                                 \
           2, 12, _Imm::Repr::Signed,                                          \
           std::vector{ImmPart(5, 25, 31), ImmPart(0, 7, 11)}),                \
       std::make_shared<_Reg>(isa, 1, 20, 24, "rs2")}))

#define UType(name, opcode)                                                    \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      _Opcode(name, {OpPart(opcode, 0, 6)}),                                   \
      {std::make_shared<_Reg>(isa, 1, 7, 11, "rd"),                            \
       std::make_shared<_Imm>(2, 32, _Imm::Repr::Hex,                          \
                              std::vector{ImmPart(0, 12, 31)})}))

#define JType(name, opcode)                                                    \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      _Opcode(name, {OpPart(opcode, 0, 6)}),                                   \
      {std::make_shared<_Reg>(isa, 1, 7, 11, "rd"),                            \
       std::make_shared<_Imm>(                                                 \
           2, 21, _Imm::Repr::Signed,                                          \
           std::vector{ImmPart(20, 31, 31), ImmPart(12, 12, 19),               \
                       ImmPart(11, 20, 20), ImmPart(1, 21, 30)},               \
           _Imm::SymbolType::Relative)}))

#define JALRType(name)                                                         \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      _Opcode(name,                                                            \
              {OpPart(RVISA::Opcode::JALR, 0, 6), OpPart(0b000, 12, 14)}),     \
      {std::make_shared<_Reg>(isa, 1, 7, 11, "rd"),                            \
       std::make_shared<_Reg>(isa, 2, 15, 19, "rs1"),                          \
       std::make_shared<_Imm>(3, 12, _Imm::Repr::Signed,                       \
                              std::vector{ImmPart(0, 20, 31)})}))

#define RegTok _PseudoInstruction::reg()
#define ImmTok _PseudoInstruction::imm()
#define Create_PseudoInstruction
#define _PseudoExpandFuncSyms(line, symbols)                                   \
  [=](const _PseudoInstruction &, const TokenizedSrcLine &line,                \
      const SymbolMap &symbols)

#define _PseudoExpandFunc(line)                                                \
  [](const _PseudoInstruction &, const TokenizedSrcLine &line,                 \
     const SymbolMap &)

#define PseudoLoad(name)                                                       \
  std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(                  \
      name, {RegTok, ImmTok}, _PseudoExpandFunc(line) {                        \
        LineTokensVec v;                                                       \
        v.push_back(LineTokens() << Token("auipc") << line.tokens.at(1)        \
                                 << Token(line.tokens.at(2), "%pcrel_hi"));    \
        v.push_back(LineTokens()                                               \
                    << name << line.tokens.at(1)                               \
                    << Token(QString("(%1 + 4)").arg(line.tokens.at(2)),       \
                             "%pcrel_lo")                                      \
                    << line.tokens.at(1));                                     \
        return v;                                                              \
      }))

// The sw is a pseudo-op if a symbol is given as the immediate token. Thus, if
// we detect that a number has been provided, then abort the pseudo-op handling.
#define PseudoStore(name)                                                      \
  std::shared_ptr<_PseudoInstruction>(new _PseudoInstruction(                  \
      name, {RegTok, ImmTok, RegTok}, _PseudoExpandFunc(line) {                \
        bool canConvert;                                                       \
        getImmediate(line.tokens.at(2), canConvert);                           \
        if (canConvert) {                                                      \
          return Result<std::vector<LineTokens>>(                              \
              Error(0, "Unused; will fallback to non-pseudo op sw"));          \
        }                                                                      \
        LineTokensVec v;                                                       \
        v.push_back(LineTokens() << Token("auipc") << line.tokens.at(3)        \
                                 << Token(line.tokens.at(2), "%pcrel_hi"));    \
        v.push_back(LineTokens()                                               \
                    << name << line.tokens.at(1)                               \
                    << Token(QString("(%1 + 4)").arg(line.tokens.at(2)),       \
                             "%pcrel_lo")                                      \
                    << line.tokens.at(3));                                     \
        return Result<std::vector<LineTokens>>(v);                             \
      }))

} // namespace Assembler
} // namespace Ripes
