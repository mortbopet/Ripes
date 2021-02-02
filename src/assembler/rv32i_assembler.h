#pragma once

#include "assembler.h"

#include <functional>

namespace Ripes {
namespace Assembler {

class RV32I_Assembler : public Assembler {
public:
    RV32I_Assembler(const ISAInfo<ISA::RV32I>* isa);

private:
    std::tuple<InstrVec, PseudoInstrVec, DirectiveVec> initInstructions(const ISAInfo<ISA::RV32I>* isa) const;

    /**
     * Extension enablers
     * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
     * the assembler.
     */
    void enableExtI(const ISAInfo<ISA::RV32I>* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions) const;
    void enableExtM(const ISAInfo<ISA::RV32I>* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions) const;
    void enableExtF(const ISAInfo<ISA::RV32I>* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions) const;

protected:
    QChar commentDelimiter() const override { return '#'; }
};

}  // namespace Assembler
}  // namespace Ripes
