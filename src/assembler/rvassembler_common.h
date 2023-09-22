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

// A B-Type RISC-V instruction
template <typename Reg_T>
class BTypeInstr : public RVInstruction<Reg_T> {
public:
  BTypeInstr(const Token &name, unsigned funct3, const ISAInfoBase *isa)
      : RVInstruction<Reg_T>(
            Opcode<Reg_T>(name,
                          {
                           OpPart(RVISA::Opcode::BRANCH, 0, 6),
                           OpPart(funct3, 12, 14)
                          }),
                          {
                            std::make_shared<Reg<Reg_T>>(isa, 1, 15, 19, "rs1"),
                            std::make_shared<Reg<Reg_T>>(isa, 2, 20, 24, "rs2"),
                            std::make_shared<Imm<Reg_T>>(
                              3, 13, Imm<Reg_T>::Repr::Signed,
                              std::vector{
                                ImmPart(12, 31, 31), ImmPart(11, 7, 7),
                                ImmPart(5, 25, 30), ImmPart(1, 8, 11)
                              },
                              Imm<Reg_T>::SymbolType::Relative
                            )
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
            Opcode<Reg_T>(name, {OpPart(opcode, 0, 6), OpPart(funct3, 12, 14)}),
            {
              std::make_shared<Reg<Reg_T>>(isa, 1, 7, 11, "rd"),
              std::make_shared<Reg<Reg_T>>(isa, 2, 15, 19, "rs1"),
              std::make_shared<Imm<Reg_T>>(3, 12, Imm<Reg_T>::Repr::Signed,
                std::vector{ImmPart(0, 20, 31)})
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
            Opcode<Reg_T>(name, {OpPart(RVISA::Opcode::LOAD, 0, 6), OpPart(funct3, 12, 14)}),
            {
              std::make_shared<Reg<Reg_T>>(isa, 1, 7, 11, "rd"),
                std::make_shared<Reg<Reg_T>>(isa, 3, 15, 19, "rs1"),
                std::make_shared<Imm<Reg_T>>(2, 12, Imm<Reg_T>::Repr::Signed,
                                          std::vector{ImmPart(0, 20, 31)})
            }) {}
};

template <typename Reg_T>
std::shared_ptr<Instruction<Reg_T>> defineLType(const QString &name, unsigned funct3,
                                                  const ISAInfoBase *isa) {
  return std::shared_ptr<Instruction<Reg_T>>(
      new LTypeInstr<Reg_T>(Token(name), funct3, isa));
}

// The following macros assumes that ASSEMBLER_TYPES(..., ...) has been defined
// for the given assembler.

#define IShiftType32(name, opcode, funct3, funct7)                             \
  std::shared_ptr<_Instruction>(new _Instruction(                              \
      _Opcode(name, {OpPart(opcode, 0, 6), OpPart(funct3, 12, 14),             \
                     OpPart(funct7, 25, 31)}),                                 \
      {std::make_shared<_Reg>(isa, 1, 7, 11, "rd"),                            \
       std::make_shared<_Reg>(isa, 2, 15, 19, "rs1"),                          \
       std::make_shared<_Imm>(3, 5, _Imm::Repr::Unsigned,                      \
                              std::vector{ImmPart(0, 20, 24)})}))

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
