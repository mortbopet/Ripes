#pragma once

#include <QObject>
#include <functional>

#include "assembler.h"
#include "rvassembler_common.h"

namespace Ripes {
namespace Assembler {

/**
 * Extension enabler.
 * Calling an extension enabler will register the appropriate assemblers and
 * pseudo-op expander functors with the assembler.
 */
struct RV_M {
  static void enable(const ISAInfoBase *isa, InstrVec &instructions,
                     PseudoInstrVec & /*pseudoInstructions*/) {
    // Pseudo-op functors

    // Assembler functors
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVInstrRType(Token("mul"), 0b000, 0b0000001, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVInstrRType(Token("mulh"), 0b001, 0b0000001, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVInstrRType(Token("mulhsu"), 0b010, 0b0000001, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVInstrRType(Token("mulhu"), 0b011, 0b0000001, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVInstrRType(Token("div"), 0b100, 0b0000001, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVInstrRType(Token("divu"), 0b101, 0b0000001, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVInstrRType(Token("rem"), 0b110, 0b0000001, isa)));
    instructions.push_back(std::shared_ptr<Instruction>(
        new RVInstrRType(Token("remu"), 0b111, 0b0000001, isa)));
  }
};

} // namespace Assembler
} // namespace Ripes
