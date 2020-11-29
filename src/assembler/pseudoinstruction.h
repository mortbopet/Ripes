#pragma once

#include "assembler_defines.h"
#include "instruction.h"

namespace Ripes {
namespace AssemblerTmp {

template <typename ISA>
class PseudoInstruction {
    static_assert(std::is_base_of<ISAInfoBase, ISA>::value, "Provided ISA type must derive from ISAInfoBase");

public:
    PseudoInstruction(
        const QString& opcode, const std::vector<std::shared_ptr<Field>>& fields,
        const std::function<PseudoExpandRes(const PseudoInstruction&, const AssemblerTmp::TokenizedSrcLine&)>& expander)
        : m_opcode(opcode), m_expectedTokens(1 /*opcode*/ + fields.size()), m_fields(fields), m_expander(expander) {}

    PseudoExpandRes expand(const AssemblerTmp::TokenizedSrcLine& line) {
        if (line.tokens.length() != m_expectedTokens) {
            return AssemblerTmp::Error(
                line.sourceLine, "Instruction '" + m_opcode + "' expects " + QString::number(m_expectedTokens - 1) +
                                     " arguments, but got " + QString::number(line.tokens.length() - 1));
        }

        return m_expander(*this, line);
    }

    const QString& name() const { return m_opcode; }

    /**
     * @brief reg() and imm() return dummy register and immediate expected tokens for a pseudo instruction.
     * Does not relate to actual regs/imms in an instruction word, but solely used for a pseudoinstruction to
     * be used within the instruction token checking system.
     */
    static std::shared_ptr<Reg<ISA>> reg() { return std::make_shared<Reg<ISA>>(0, 0, 0); }
    static std::shared_ptr<Imm> imm() { return std::make_shared<Imm>(0, 0, Imm::Repr::Hex, std::vector<ImmPart>{}); }

private:
    std::function<PseudoExpandRes(const PseudoInstruction& /*this*/, const AssemblerTmp::TokenizedSrcLine&)> m_expander;

    const QString m_opcode;
    const int m_expectedTokens;
    const std::vector<std::shared_ptr<Field>> m_fields;
};

}  // namespace AssemblerTmp
}  // namespace Ripes
