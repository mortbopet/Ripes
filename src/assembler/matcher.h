#pragma once

#include <iostream>
#include <memory>
#include <numeric>

#include "isa/instruction.h"

namespace Ripes {
namespace Assembler {

static std::shared_ptr<OpPartBase> BaseMatcher =
    std::make_shared<OpPart<0, BitRange<0, 0>>>();

class Matcher {
  class MatchNode {
  private:
    /// If set, this match node will base its matching on the extra conditions
    /// for a function. Else, matches on the OpPart assoacited with this node.
    bool m_matchOnExtraMatchConds = false;

  public:
    MatchNode(const OpPartBase *_match) : match(_match) {}
    const OpPartBase *match;
    std::vector<MatchNode> children;
    std::shared_ptr<InstructionBase> instruction;
    void matchOnExtraMatchConds() { m_matchOnExtraMatchConds = true; }

    bool matches(const Instr_T &instr) const {
      return m_matchOnExtraMatchConds ? instruction->matchesWithExtras(instr)
                                      : match->matches(instr);
    }

    void print(unsigned depth = 0) const {
      if (depth == 0) {
        std::cout << "root";
      } else {
        for (unsigned i = 0; i < depth; ++i) {
          std::cout << "-";
        }

        if (!instruction ||
            (instruction && depth <= instruction->numOpParts())) {
          QString matchField =
              QString::number(match->range().stop()) + "[" +
              QStringLiteral("%1").arg(match->value(), match->range().width(),
                                       2, QLatin1Char('0')) +
              "]" + QString::number(match->range().start());
          std::cout << matchField.toStdString();
        } else {
          // Extra match conditions must apply
          assert(instruction.get()->hasExtraMatchConds());
        }
        std::cout << " -> ";
      }

      if (instruction) {
        std::cout << instruction.get()->name().toStdString();
        if (instruction.get()->hasExtraMatchConds())
          std::cout << "*";
        std::cout << std::endl;
      } else {
        std::cout << std::endl;
        for (const auto &child : children) {
          child.print(depth + 1);
        }
      }
    }
  };

public:
  Matcher(const std::vector<std::shared_ptr<InstructionBase>> &instructions)
      : m_matchRoot(buildMatchTree(instructions)) {}
  void print() const { m_matchRoot.print(); }

  Result<const InstructionBase *>
  matchInstruction(const Instr_T &instruction) const {
    auto match = matchInstructionRec(instruction, m_matchRoot, true);
    if (match == nullptr) {
      return Error(0, "Unknown instruction");
    }
    return match;
  }

private:
  const InstructionBase *matchInstructionRec(const Instr_T &instruction,
                                             const MatchNode &node,
                                             bool isRoot) const {
    if (isRoot || node.matches(instruction)) {
      if (node.children.size() > 0) {
        for (const auto &child : node.children) {
          if (auto matchedInstr =
                  matchInstructionRec(instruction, child, false);
              matchedInstr != nullptr) {
            return matchedInstr;
          }
        }
      } else if (node.instruction->matchesWithExtras(instruction)) {
        return &(*node.instruction);
      }
    }
    return nullptr;
  }

  MatchNode buildMatchTree(const InstrVec &instructions,
                           unsigned fieldDepth = 1,
                           const OpPartBase *match = BaseMatcher.get()) {
    std::map<OpPartMatcher, InstrVec> instrsWithEqualOpPart;
    for (const auto &instr : instructions) {
      if (auto instrRef = instr.get()) {
        const size_t nOpParts = instrRef->numOpParts();
        if (nOpParts < fieldDepth && !instrRef->hasExtraMatchConds()) {
          QString err = "Instruction '" + instr->name() +
                        "' cannot be decoded; aliases with other instruction "
                        "(Needs more discernable parts)\n";
          throw std::runtime_error(err.toStdString().c_str());
        }
        const OpPartBase *opPart;
        if (fieldDepth > nOpParts)
          opPart = instrRef->getOpPart(nOpParts - 1);
        else
          opPart = instrRef->getOpPart(fieldDepth - 1);
        if (nOpParts == fieldDepth &&
            instrsWithEqualOpPart.count(opPart->getMatcher()) != 0) {
          QString err;
          err += "Instruction cannot be decoded; aliases with other "
                 "instruction (Identical to other "
                 "instruction)\n";
          err += instr->name() + " is equal to " +
                 instrsWithEqualOpPart.at(opPart->getMatcher()).at(0)->name();
          throw std::runtime_error(err.toStdString().c_str());
        }
        instrsWithEqualOpPart[opPart->getMatcher()].push_back(instr);
      }
    }

    MatchNode node(match);
    for (const auto &iter : instrsWithEqualOpPart) {
      bool isUniqueIdentifiable = false;
      if (iter.second.size() == 1) {
        auto &instr = iter.second[0];
        const size_t nOpParts = instr->numOpParts();
        if (fieldDepth == nOpParts || instr->hasExtraMatchConds()) {
          // End of opParts, uniquely identifiable instruction
          MatchNode child(iter.first.opPart);
          // At this point, the opPart at this level for the instruction is
          // invalid, and we must match on extra conditions.
          if (fieldDepth > nOpParts) {
            assert(instr->hasExtraMatchConds());
            child.matchOnExtraMatchConds();
          }
          child.instruction = instr;
          node.children.push_back(child);
          isUniqueIdentifiable = true;
        } else {
          // More opParts available; need to match on these as well. It might be
          // that different instructions alias across non-opcode fields such as
          // immediates or registers. If the ISA is valid, matching on all
          // opcode parts should yield the correct instruction.
        }
      }

      if (!isUniqueIdentifiable) {
        // Match branch; recursively continue match tree
        node.children.push_back(
            buildMatchTree(iter.second, fieldDepth + 1, iter.first.opPart));
      }
    }

    return node;
  }

  MatchNode m_matchRoot;
};

} // namespace Assembler
} // namespace Ripes
