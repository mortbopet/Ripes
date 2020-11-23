#pragma once

#include "assembler.h"

#include <functional>

namespace Ripes {
namespace AssemblerTmp {

class RV32I_Assembler : public AssemblerBase<ISAInfo<ISA::RV32IM>> {
private:
    using RVInstr = Instruction<ISAInfo<ISA::RV32IM>>;
    using RVInstrVec = std::vector<std::shared_ptr<RVInstr>>;
    using RVPseudoInstr = PseudoInstruction<ISAInfo<ISA::RV32IM>>;
    using RVPseudoInstrVec = std::vector<std::shared_ptr<RVPseudoInstr>>;

public:
    enum class Extensions { M, F };
    RV32I_Assembler(const std::set<Extensions>& extensions);

public:
    std::pair<RVInstrVec, RVPseudoInstrVec> initInstructions(const std::set<Extensions>& extensions) const;

    /**
     * Extension enablers
     * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
     * the assembler.
     */
    void enableExtI(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const;
    void enableExtM(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const;
    void enableExtF(RVInstrVec& instructions, RVPseudoInstrVec& pseudoInstructions) const;
};

}  // namespace AssemblerTmp
}  // namespace Ripes
