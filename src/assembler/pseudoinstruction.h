#pragma once

#include "assembler_defines.h"
#include "instruction.h"

namespace Ripes {
namespace Assembler {

template <typename Reg_T>
class PseudoInstruction;

template <typename Reg_T>
using PseudoExpandFunc =
    std::function<PseudoExpandRes(const PseudoInstruction<Reg_T>& /*this*/, const TokenizedSrcLine&, const SymbolMap&)>;

template <typename Reg_T>
class PseudoInstruction {
public:
    PseudoInstruction(const QString& opcode, const std::vector<std::shared_ptr<Field<Reg_T>>>& fields,
                      const PseudoExpandFunc<Reg_T>& expander)
        : m_expander(expander), m_opcode(opcode), m_expectedTokens(1 /*opcode*/ + fields.size()), m_fields(fields) {}

    PseudoExpandRes expand(const TokenizedSrcLine& line, const SymbolMap& symbols) {
        if (line.tokens.length() != m_expectedTokens) {
            return Error(line.sourceLine, "Instruction '" + m_opcode + "' expects " +
                                              QString::number(m_expectedTokens - 1) + " arguments, but got " +
                                              QString::number(line.tokens.length() - 1));
        }

        return m_expander(*this, line, symbols);
    }

    const QString& name() const { return m_opcode; }

    /**
     * @brief reg() and imm() return dummy register and immediate expected tokens for a pseudo instruction.
     * Does not relate to actual regs/imms in an instruction word, but solely used for a pseudoinstruction to
     * be used within the instruction token checking system.
     */
    static std::shared_ptr<Reg<Reg_T>> reg() { return std::make_shared<Reg<Reg_T>>(nullptr, 0, 0, 0, "rd"); }
    static std::shared_ptr<Imm<Reg_T>> imm() {
        return std::make_shared<Imm<Reg_T>>(0, 0, Imm<Reg_T>::Repr::Hex, std::vector<ImmPart>{});
    }

private:
    PseudoExpandFunc<Reg_T> m_expander;

    const QString m_opcode;
    const int m_expectedTokens;
    const std::vector<std::shared_ptr<Field<Reg_T>>> m_fields;
};

#define PseudoExpandFunc(line, symbols) \
    [](const PseudoInstruction<Reg_T>&, const TokenizedSrcLine& line, const SymbolMap& symbols)

template <typename Reg_T>
using PseudoInstrVec = std::vector<std::shared_ptr<PseudoInstruction<Reg_T>>>;

template <typename Reg_T>
using PseudoInstrMap = std::map<QString, std::shared_ptr<PseudoInstruction<Reg_T>>>;

}  // namespace Assembler
}  // namespace Ripes
