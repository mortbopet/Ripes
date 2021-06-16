#pragma once

#include "assembler.h"

#include <QObject>
#include <functional>

namespace Ripes {
namespace Assembler {

class RV32I_Assembler : public QObject, public Assembler {
    Q_OBJECT
public:
    RV32I_Assembler(const ISAInfo<ISA::RV32I>* isa);

private:
    std::tuple<InstrVec, PseudoInstrVec> initInstructions(const ISAInfo<ISA::RV32I>* isa) const;

    /**
     * Extension enablers
     * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
     * the assembler.
     */
    static void enableExtI(const ISAInfoBase* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions);
    static void enableExtM(const ISAInfoBase* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions);
    static void enableExtF(const ISAInfoBase* isa, InstrVec& instructions, PseudoInstrVec& pseudoInstructions);

protected:
    QChar commentDelimiter() const override { return '#'; }
};

}  // namespace Assembler
}  // namespace Ripes
