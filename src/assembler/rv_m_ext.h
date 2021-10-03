#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

/**
 * Extension enabler.
 * Calling an extension enabler will register the appropriate assemblers and pseudo-op expander functors with
 * the assembler.
 * The extension enablers are templated to allow for sharing implementations between 32- and 64-bit variants.
 */
template <typename Reg__T>
struct RV_M {
    AssemblerTypes(Reg__T);
    static void enable(const ISAInfoBase* isa, _InstrVec& instructions, _PseudoInstrVec& /*pseudoInstructions*/) {
        // Pseudo-op functors

        // Assembler functors
        instructions.push_back(RType(Token("mul"), 0b000, 0b0000001));
        instructions.push_back(RType(Token("mulh"), 0b001, 0b0000001));
        instructions.push_back(RType(Token("mulhsu"), 0b010, 0b0000001));
        instructions.push_back(RType(Token("mulhu"), 0b011, 0b0000001));
        instructions.push_back(RType(Token("div"), 0b100, 0b0000001));
        instructions.push_back(RType(Token("divu"), 0b101, 0b0000001));
        instructions.push_back(RType(Token("rem"), 0b110, 0b0000001));
        instructions.push_back(RType(Token("remu"), 0b111, 0b0000001));
    }
};

}  // namespace Assembler
}  // namespace Ripes
