#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

#define CAType(name, funct2, funct6)                                                              \
    std::shared_ptr<_Instruction>(new _Instruction(                                               \
        Opcode<Reg__T>(name, {OpPart(0b01, 0, 1), OpPart(funct2, 5, 6), OpPart(funct6, 10, 15)}), \
        {std::make_shared<_Reg>(isa, 2, 2, 4, "rs2'"), std::make_shared<_Reg>(isa, 1, 7, 9, "rd'/rs1'")}))

/**
 * Extension enabler.
 * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
 * the assembler.
 * The extension enablers are templated to allow for sharing implementations between 32- and 64-bit variants.
 */
template <typename Reg__T>
struct RV_C {
    ASSEMBLER_TYPES(Reg__T)
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
