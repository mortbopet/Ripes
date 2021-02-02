#pragma once

#include <iostream>
#include <memory>
#include "instruction.h"

namespace Ripes {
namespace Assembler {

class Matcher {
    struct MatchNode {
        MatchNode(OpPart _matcher) : matcher(_matcher) {}
        OpPart matcher;
        std::vector<MatchNode> children;
        std::shared_ptr<Instruction> instruction;

        void print(unsigned depth = 0) const {
            if (depth == 0) {
                std::cout << "root";
            } else {
                for (int i = 0; i < depth; i++) {
                    std::cout << "-";
                }
                QString matchField =
                    QString::number(matcher.range.stop) + "[" +
                    QStringLiteral("%1").arg(matcher.value, matcher.range.width(), 2, QLatin1Char('0')) + "]" +
                    QString::number(matcher.range.start);
                std::cout << matchField.toStdString() << " -> ";
            }

            if (instruction) {
                std::cout << instruction.get()->name().toStdString() << std::endl;
            } else {
                std::cout << std::endl;
                for (const auto& child : children) {
                    child.print(depth + 1);
                }
            }
        }
    };

public:
    Matcher(const std::vector<std::shared_ptr<Instruction>>& instructions)
        : m_matchRoot(buildMatchTree(instructions, 1)) {}
    void print() const { m_matchRoot.print(); }

    std::variant<Error, const Instruction*> matchInstruction(uint32_t instruction) const {
        auto match = matchInstructionRec(instruction, m_matchRoot, true);
        if (match == nullptr) {
            return Error(0, "Unknown instruction");
        }
        return match;
    }

private:
    const Instruction* matchInstructionRec(uint32_t instruction, const MatchNode& node, bool isRoot) const {
        if (isRoot || node.matcher.matches(instruction)) {
            if (node.children.size() > 0) {
                for (const auto& child : node.children) {
                    if (auto matchedInstr = matchInstructionRec(instruction, child, false)) {
                        return matchedInstr;
                    }
                }
            } else {
                return &(*node.instruction);
            }
        }
        return nullptr;
    }

    MatchNode buildMatchTree(const std::vector<std::shared_ptr<Instruction>>& instructions,
                             const unsigned fieldDepth = 1, OpPart matcher = OpPart(0, BitRange(0, 0, 2))) {
        using InstrVec = std::vector<std::shared_ptr<Instruction>>;
        std::map<OpPart, InstrVec> instrsWithEqualOpPart;

        for (const auto& instr : instructions) {
            if (auto instrRef = instr.get()) {
                const size_t nOpParts = instrRef->getOpcode().opParts.size();
                if (nOpParts < fieldDepth) {
                    assert(
                        false &&
                        "Instruction cannot be decoded; aliases with other instruction (Needs more discernable parts)");
                }
                const OpPart opPart = instrRef->getOpcode().opParts[fieldDepth - 1];
                if (nOpParts == fieldDepth && instrsWithEqualOpPart.count(opPart) != 0) {
                    assert(false &&
                           "Instruction cannot be decoded; aliases with other instruction (Identical to other "
                           "instruction)");
                }
                instrsWithEqualOpPart[opPart].push_back(instr);
            }
        }

        MatchNode node(matcher);
        for (const auto& iter : instrsWithEqualOpPart) {
            if (iter.second.size() == 1) {
                // Uniquely identifiable instruction
                MatchNode child(iter.first);
                child.instruction = iter.second[0];
                node.children.push_back(child);
            } else {
                // Match branch; recursively continue match tree
                node.children.push_back(buildMatchTree(iter.second, fieldDepth + 1, iter.first));
            }
        }

        return node;
    }

    MatchNode m_matchRoot;
};

}  // namespace Assembler
}  // namespace Ripes
