#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "instruction.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

/// Register specialization for RV-C to handle the compressed register notation. 3 bits are used to represent registers
/// x8-x15, with register x5=0b000, x15=0b111.
template <typename Reg_T>
struct RVCReg : public Reg<Reg_T> {
    using Reg<Reg_T>::Reg;
    std::optional<Error> apply(const TokenizedSrcLine& line, Instr_T& instruction,
                               FieldLinkRequest<Reg_T>&) const override {
        bool success;
        const QString& regToken = line.tokens[this->tokenIndex];
        unsigned reg = this->m_isa->regNumber(regToken, success);
        if (!success)
            return Error(line.sourceLine, "Unknown register '" + regToken + "'");
        if (!(8 <= reg && reg <= 15))
            return Error(line.sourceLine, "Only registers x8-x15 are allowed for this instruction");
        reg -= 8;
        instruction |= this->m_range.apply(reg);
        return std::nullopt;
    }

    std::optional<Error> decode(const Instr_T instruction, const Reg_T /*address*/, const ReverseSymbolMap&,
                                LineTokens& line) const override {
        const unsigned regNumber = this->m_range.decode(instruction) + 8;
        const Token registerName = this->m_isa->regName(regNumber);
        if (registerName.isEmpty()) {
            return Error(0, "Unknown register number '" + QString::number(regNumber) + "'");
        }
        line.push_back(registerName);
        return std::nullopt;
    }
};

#define CAType(name, funct2, funct6)                                                                               \
    std::shared_ptr<_Instruction>(                                                                                 \
        new _Instruction(Opcode<Reg__T>(name, {OpPart(0b01, 0, 1), OpPart(funct2, 5, 6), OpPart(funct6, 10, 15)}), \
                         {std::make_shared<RVCReg<Reg__T>>(isa, 2, 2, 4, "rs2'"),                                  \
                          std::make_shared<RVCReg<Reg__T>>(isa, 1, 7, 9, "rd'/rs1'")}))

/**
 * Extension enabler.
 * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
 * the assembler.
 * The extension enablers are templated to allow for sharing implementations between 32- and 64-bit variants.
 */
template <typename Reg__T>
struct RV_C {
    AssemblerTypes(Reg__T);
    static void enable(const ISAInfoBase* isa, _InstrVec& instructions, _PseudoInstrVec& /*pseudoInstructions*/) {
        // Pseudo-op functors

        // Assembler functors
        instructions.push_back(CAType(Token("c.and"), 0b11, 0b100011));
        instructions.push_back(CAType(Token("c.subw"), 0b00, 0b100111));
        instructions.push_back(CAType(Token("c.addw"), 0b01, 0b100111));
        instructions.push_back(CAType(Token("c.or"), 0b10, 0b100011));
        instructions.push_back(CAType(Token("c.xor"), 0b01, 0b100011));
        instructions.push_back(CAType(Token("c.sub"), 0b00, 0b100011));
    }
};

}  // namespace Assembler
}  // namespace Ripes
