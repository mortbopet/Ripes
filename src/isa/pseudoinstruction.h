#pragma once

#include <optional>

#include "instruction.h"
#include "isa_defines.h"
#include "symbolmap.h"

namespace Ripes {

struct PseudoInstructionBase {
  virtual Result<std::vector<LineTokens>> expand(const TokenizedSrcLine &line,
                                                 SymbolMap &symbols) = 0;
  virtual QString name() const = 0;
};

using PseudoExpandFunc = std::function<Result<std::vector<LineTokens>>(
    const PseudoInstructionBase & /* this */, const TokenizedSrcLine &,
    const SymbolMap &)>;

template <typename PseudoInstrImpl>
struct PseudoInstruction : public PseudoInstructionBase {
  QString name() const override { return PseudoInstrImpl::mnemonic(); }
  Result<std::vector<LineTokens>> expand(const TokenizedSrcLine &line,
                                         SymbolMap &symbols) override {
    if (line.tokens.length() != PseudoInstrImpl::ExpectedTokens) {
      return Error(line,
                   "Instruction '" + name() + "' expects " +
                       QString::number(PseudoInstrImpl::ExpectedTokens - 1) +
                       " arguments, but got " +
                       QString::number(line.tokens.length() - 1));
    }

    return PseudoInstrImpl::expander(*this, line, symbols);
  }
};

template <unsigned index, typename ISAImpl>
struct PseudoReg : public Reg<index, BitRange<index, index>, ISAImpl> {
  PseudoReg() : Reg<index, BitRange<index, index>, ISAImpl>("rd") {}
};

template <unsigned index>
struct PseudoImm
    : public Imm<index, 0, Repr::Hex, ImmPart<0, BitRange<index, index>>> {};

using PseudoInstrVec = std::vector<std::shared_ptr<PseudoInstructionBase>>;

using PseudoInstrMap =
    std::map<QString, std::shared_ptr<PseudoInstructionBase>>;

} // namespace Ripes
