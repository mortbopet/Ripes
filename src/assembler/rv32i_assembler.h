#pragma once

#include "assembler.h"

#include <functional>

namespace Ripes {
namespace AssemblerTmp {

class RV32I_Assembler : public Assembler<ISAInfo<ISA::RV32IM>> {
private:
    using RVInstr = Instruction<ISAInfo<ISA::RV32IM>>;
    using RVInstrVec = std::vector<std::shared_ptr<RVInstr>>;
    using RVPseudoInstr = PseudoInstruction<ISAInfo<ISA::RV32IM>>;
    using RVPseudoInstrVec = std::vector<std::shared_ptr<RVPseudoInstr>>;
    using RVReg = Reg<ISAInfo<ISA::RV32IM>>;

public:
    enum class Extensions { M, F };
    RV32I_Assembler(const std::set<Extensions> extensions = std::set<Extensions>());

public:
    std::tuple<RVInstrVec, RVPseudoInstrVec, DirectiveVec>
    initInstructions(const std::set<Extensions>& extensions) const;

    /**
     * Extension enablers
     * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
     * the assembler.
     */
    void enableExtI(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const;
    void enableExtM(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const;
    void enableExtF(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const;

protected:
    QChar commentDelimiter() const override { return '#'; }
};

}  // namespace AssemblerTmp
}  // namespace Ripes
