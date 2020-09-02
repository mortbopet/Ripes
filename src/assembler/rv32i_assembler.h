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

protected:
    std::variant<Error, std::optional<std::vector<LineTokens>>> expandPseudoOp(const SourceLine& line) const override;
    std::variant<Error, QByteArray> assembleInstruction(const SourceLine& line,
                                                        const SymbolMap& symbols) const override;

public:
    /**
     * @brief m_pseudoOpExpanders
     * A map of (pseudo)opcodes => pseudo-op expander functors
     */
    using PseudoOpFunctorRetT = std::optional<std::vector<LineTokens>>;
    using PseudoOpFunctor = std::function<std::variant<Error, PseudoOpFunctorRetT>(const SourceLine&)>;
    std::map<QString, PseudoOpFunctor> m_pseudoOpExpanders;
    void addPseudoOpExpanderFunctor(const QString& opcode, PseudoOpFunctor functor);

    /**
     * @brief m_assemblers
     * a map of opcodes => instruction assembler functors
     */
    using AssemblerFunctorRetT = QByteArray;
    using AssemblerFunctor =
        std::function<std::variant<Error, AssemblerFunctorRetT>(const SourceLine&, const SymbolMap&)>;
    std::map<QString, AssemblerFunctor> m_assemblers;
    void addAssemblerFunctor(const QString& opcode, AssemblerFunctor functor);

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
