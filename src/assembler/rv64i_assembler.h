#pragma once

#include "assembler.h"

#include <QObject>
#include <functional>

#include "rv_i_ext.h"
#include "rv_m_ext.h"

namespace Ripes {
namespace Assembler {

class RV64I_Assembler : public QObject, public Assembler<uint64_t, uint32_t> {
    Q_OBJECT

    using Reg_T = uint64_t;
    using Instr_T = uint32_t;
    ASSEMBLER_TYPES(Reg_T, Instr_T)

public:
    RV64I_Assembler(const ISAInfo<ISA::RV64I>* isa);

private:
    std::tuple<_InstrVec, _PseudoInstrVec> initInstructions(const ISAInfo<ISA::RV64I>* isa) const;

    /**
     * Extension enablers
     * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
     * the assembler.
     */
    static void enableExtI(const ISAInfoBase* isa, _InstrVec& instructions, _PseudoInstrVec& pseudoInstructions);
    static void enableExtM(const ISAInfoBase* isa, _InstrVec& instructions, _PseudoInstrVec& pseudoInstructions);

protected:
    QChar commentDelimiter() const override { return '#'; }
};

}  // namespace Assembler
}  // namespace Ripes
