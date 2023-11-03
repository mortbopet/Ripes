#pragma once

#include <optional>

#include "instruction.h"
#include "isa_defines.h"
#include "symbolmap.h"

namespace Ripes {

struct PseudoInstructionBase {
  virtual Result<std::vector<LineTokens>> expand(const TokenizedSrcLine &line,
                                                 SymbolMap &symbols) const = 0;
  virtual QString name() const = 0;
};

using PseudoExpandFunc = std::function<Result<std::vector<LineTokens>>(
    const PseudoInstructionBase & /* this */, const TokenizedSrcLine &,
    const SymbolMap &)>;

template <typename PseudoInstrImpl>
struct PseudoInstruction : public PseudoInstructionBase {
  constexpr static unsigned ExpectedTokens() {
    return 1 + PseudoInstrImpl::Fields::NumFields();
  }
  QString name() const override {
    return QString(PseudoInstrImpl::Name.data());
  }
  Result<std::vector<LineTokens>> expand(const TokenizedSrcLine &line,
                                         SymbolMap &symbols) const override {
    if (line.tokens.length() != ExpectedTokens()) {
      return Error(line, "Instruction '" + name() + "' expects " +
                             QString::number(ExpectedTokens() - 1) +
                             " arguments, but got " +
                             QString::number(line.tokens.length() - 1));
    }

    return PseudoInstrImpl::expander(*this, line, symbols);
  }
};

template <unsigned index, typename ISAImpl>
struct PseudoReg : public Reg<PseudoReg<index, ISAImpl>, index,
                              BitRange<index, index>, ISAImpl> {
  constexpr static std::string_view Name = "rd";
};

template <unsigned index>
struct PseudoImm : public Imm<index, 1, Repr::Hex, ImmPart<0, index, index>> {};

using PseudoInstrVec = std::vector<std::shared_ptr<PseudoInstructionBase>>;

using PseudoInstrMap =
    std::map<QString, std::shared_ptr<PseudoInstructionBase>>;

template <typename... Instructions>
constexpr inline static void
enablePseudoInstructions(PseudoInstrVec &instructions) {
  // TODO(raccog): Ensure no duplicate pseudo-instruction definitions
  // TODO(raccog): Verify instructions generated from pseudoinstructions
  return _enableInstructions<PseudoInstrVec, Instructions...>(instructions);
}

} // namespace Ripes
