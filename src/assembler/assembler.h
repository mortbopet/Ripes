#pragma once

#include <QRegularExpression>

#include "assembler_defines.h"
#include "isainfo.h"
#include "lexerutilities.h"
#include "matcher.h"

#include <set>
#include <variant>

namespace Ripes {
namespace AssemblerTmp {

/**
 * A macro for running an assembler pass with error handling
 */
#define runPass(resName, resType, passFunction, ...)                             \
    auto passFunction##_res = passFunction(__VA_ARGS__);                         \
    try {                                                                        \
        auto& errors = std::get<Errors>(passFunction##_res);                     \
        result.errors.insert(result.errors.end(), errors.begin(), errors.end()); \
        return result;                                                           \
    } catch (const std::bad_variant_access&) {                                   \
    }                                                                            \
    auto resName = std::get<resType>(passFunction##_res);

/**
 * A macro for running an ISA-specific assembler instruction with error handling
 */
#define runOperation(resName, resType, operationFunction, ...)      \
    auto operationFunction##_res = operationFunction(__VA_ARGS__);  \
    try {                                                           \
        errors.push_back(std::get<Error>(operationFunction##_res)); \
        continue;                                                   \
    } catch (const std::bad_variant_access&) {                      \
    }                                                               \
    auto resName = std::get<resType>(operationFunction##_res);

template <typename ISA>
class AssemblerBase {
    using Instr = Instruction<ISA>;
    using InstrMap = std::map<QString, std::shared_ptr<Instr>>;
    using InstrVec = std::vector<std::shared_ptr<Instr>>;
    using PseudoInstr = PseudoInstruction<ISA>;
    using PseudoInstrMap = std::map<QString, std::shared_ptr<PseudoInstr>>;
    using PseudoInstrVec = std::vector<std::shared_ptr<PseudoInstr>>;

public:
    Result assemble(const QString& program) const {
        const auto programLines = program.split(QRegExp("[\r\n]"));
        return assemble(programLines);
    }
    Result assemble(const QStringList& programLines) const {
        Result result;

        // Tokenize each source line and separate symbol from remainder of tokens
        runPass(tokenizedLines, Program, pass0, programLines);

        // Pseudo instruction expansion
        runPass(expandedLines, Program, pass1, tokenizedLines);

        // Assemble & generate symbol map
        SymbolMap symbolMap;
        runPass(program, QByteArray, pass2, expandedLines, symbolMap);

        // Symbol linkage
        runPass(unused, NoPassResult, pass3, program, symbolMap);
        result.program = program;
        return result;
    }
    QString disassemble(const QByteArray& program) const;

    const Matcher<ISA>& getMatcher() { return m_matcher; }

private:
    /**
     * @brief pass0
     * Line tokenization and source line recording
     */
    std::variant<Errors, Program> pass0(const QStringList& program) const {
        Errors errors;
        Program tokenizedLines;
        tokenizedLines.reserve(program.size());

        /** @brief carry
         * A symbol should refer to the next following assembler line; whether an instruction or directive.
         * The carry is used to carry over symbol definitions from empty lines onto the next valid line.
         */
        Symbols carry;
        for (unsigned i = 0; i < program.size(); i++) {
            const auto& line = program.at(i);
            if (line.isEmpty())
                continue;
            TokenizedSrcLine tsl;
            runOperation(tokens, LineTokens, tokenize, program[i]);

            runOperation(directivesAndRest, DirectivesLinePair, splitDirectivesFromLine, tokens, i);
            tsl.directives = directivesAndRest.first;

            runOperation(symbolsAndRest, SymbolLinePair, splitSymbolsFromLine, directivesAndRest.second, i);
            tsl.sourceLine = i;
            tsl.symbols = symbolsAndRest.first;
            tsl.tokens = symbolsAndRest.second;

            if (!tsl.symbols.empty() && tsl.tokens.empty()) {
                carry.insert(tsl.symbols.begin(), tsl.symbols.end());
            } else {
                if (!tsl.tokens.empty()) {
                    tsl.symbols.insert(carry.begin(), carry.end());
                    carry.clear();
                }
                tokenizedLines.push_back(tsl);
            }
        }
        if (errors.size() != 0) {
            return {errors};
        } else {
            return {tokenizedLines};
        }
    }

    /**
     * @brief pass1
     * Pseudo-op expansion. If @return errors is empty, pass succeeded.
     */
    std::variant<Errors, Program> pass1(const Program& tokenizedLines) const {
        Errors errors;
        Program expandedLines;
        expandedLines.reserve(tokenizedLines.size());

        for (unsigned i = 0; i < tokenizedLines.size(); i++) {
            const auto& tokenizedLine = tokenizedLines.at(i);
            runOperation(expandedOps, std::optional<std::vector<LineTokens>>, expandPseudoOp, tokenizedLine);
            if (expandedOps) {
                /** @note: Original source line is kept for all resulting lines after pseudo-op expantion.
                 * Labels, directives and comments are only kept for the first expanded op.
                 */
                const auto& eops = expandedOps.value();
                for (int j = 0; j < eops.size(); j++) {
                    TokenizedSrcLine tsl;
                    tsl.tokens = eops.at(j);
                    tsl.sourceLine = tokenizedLine.sourceLine;
                    if (j == 0) {
                        tsl.comments = tokenizedLine.comments;
                        tsl.directives = tokenizedLine.directives;
                        tsl.symbols = tokenizedLine.symbols;
                    }
                    expandedLines.push_back(tsl);
                }
            } else {
                // This was not a pseudoinstruction; just add line to the set of expanded lines
                expandedLines.push_back(tokenizedLine);
            }
        }

        if (errors.size() != 0) {
            return {errors};
        } else {
            return {expandedLines};
        }
    }

    /**
     * @brief pass2
     * Record output address and symbol positions in expanded program. If @return errors is empty, pass succeeded.
     */
    std::variant<Errors, SymbolMap> pass2(Program& tokenizedLines) const {
        Errors errors;
        SymbolMap sm;

        // Unless otherwise specified, start at instruction segment
        uint32_t address;
        try {
            address = m_segmentPointers.at(dataSegment());
        } catch (std::out_of_range const&) {
            errors.push_back(Error(-1, "No base address specified for segment: " + instrSegment()));
            return {errors};
        }

        for (auto& line : tokenizedLines) {
        }

        if (errors.size() != 0) {
            return {errors};
        } else {
            return {sm};
        }
    }

    /**
     * @brief pass2
     * Machine code translation. If @return errors is empty, pass succeeded.
     */
    std::variant<Errors, QByteArray> pass2(const Program& tokenizedLines, SymbolMap& symbolMap) const {
        QByteArray program;
        Errors errors;
        for (const auto& line : tokenizedLines) {
            runOperation(directiveBytes, std::optional<QByteArray>, assembleDirective, line);
            if (!directiveBytes) {
                runOperation(machineCode, uint32_t, assembleInstruction, line);
                program.append(QByteArray::number(machineCode));
            } else {
                program.append(directiveBytes.value());
            }
        }
        if (errors.size() != 0) {
            return {errors};
        } else {
            return {program};
        }
    }

    std::variant<Errors, NoPassResult> pass3(QByteArray& program, const SymbolMap& symbolMap) const {
        return NoPassResult();
    }

protected:
    AssemblerBase<ISA>(std::pair<InstrVec, PseudoInstrVec> instructions)
        : m_instructions(instructions.first), m_pseudoInstructions(instructions.second), m_matcher(m_instructions) {
        for (const auto& iter : m_instructions) {
            const auto instr_name = iter.get()->name();
            if (m_instructionMap.count(instr_name) != 0) {
                throw std::runtime_error("Error: instruction with opcode " + instr_name.toStdString() +
                                         " has already been registerred.");
            }
            m_instructionMap[instr_name] = iter;
        }
        for (const auto& iter : m_pseudoInstructions) {
            const auto instr_name = iter.get()->name();
            if (m_pseudoInstructionMap.count(instr_name) != 0) {
                throw std::runtime_error("Error: pseudo-instruction with opcode " + instr_name.toStdString() +
                                         " has already been registerred.");
            }
            m_pseudoInstructionMap[instr_name] = iter;
        }
    }

    virtual PseudoExpandRes expandPseudoOp(const TokenizedSrcLine& line) const {
        if (line.tokens.empty()) {
            return std::optional<std::vector<AssemblerTmp::LineTokens>>{};
        }
        const auto& opcode = line.tokens.at(0);
        if (m_pseudoInstructionMap.count(opcode) == 0) {
            // Not a pseudo instruction
            return std::optional<std::vector<AssemblerTmp::LineTokens>>{};
        }
        return m_pseudoInstructionMap.at(opcode)->expand(line);
    }

    virtual std::variant<Error, LineTokens> tokenize(const QString& line) const {
        const static auto splitter = QRegularExpression(
            R"(\t|\((?=x(?:[1-2]\d|3[0-1]|\d)|t[0-6]|a[0-7]|s(?:1[0-1]|\d)|[sgt]p|zero)|(?:x(?:[1-2]\d|3[0-1]|\d)|t[0-6]|a[0-7]|s(?:1[0-1]|\d)|[sgt]p|zero)\K\))");
        return splitQuotes(line.split(splitter));
    }

    virtual AssembleRes assembleInstruction(const TokenizedSrcLine& line) const {
        if (line.tokens.empty()) {
            return {Error(line.sourceLine, "Empty source lines should be impossible at this point")};
        }
        const auto& opcode = line.tokens.at(0);
        if (m_instructionMap.count(opcode) == 0) {
            return {Error(line.sourceLine, "Unknown opcode '" + opcode + "'")};
        }
        return m_instructionMap.at(opcode)->assemble(line);
    };

    virtual AssembleDirectiveRes assembleDirective(const TokenizedSrcLine& line) const {
        // @todo: add assembler directive assembling
        return std::optional<QByteArray>();
    };
    /**
     * @brief splitSymbolsFromLine
     * @returns a pair consisting of a symbol and the the input @p line tokens where the symbol has been removed.
     */
    virtual std::variant<Error, SymbolLinePair> splitSymbolsFromLine(const LineTokens& tokens, int sourceLine) const {
        if (tokens.size() == 0) {
            return std::pair<Symbols, LineTokens>({}, tokens);
        }

        LineTokens remainingTokens;
        remainingTokens.reserve(tokens.size());
        Symbols symbols;
        bool symbolStillAllowed = true;
        for (const auto& token : tokens) {
            if (token.contains(':')) {
                if (symbolStillAllowed) {
                    symbols.insert(QString(token).remove(':'));
                } else {
                    return Error(sourceLine, "Stray ':' in line");
                }
            } else {
                remainingTokens.append(token);
                symbolStillAllowed = false;
            }
        }
        return std::pair<Symbols, LineTokens>(symbols, remainingTokens);
    }

    virtual std::variant<Error, DirectivesLinePair> splitDirectivesFromLine(const LineTokens& tokens,
                                                                            int sourceLine) const {
        if (tokens.size() == 0) {
            return std::pair<Directives, LineTokens>({}, tokens);
        }

        LineTokens remainingTokens;
        remainingTokens.reserve(tokens.size());
        Directives directives;
        bool directivesStillAllowed = true;
        for (const auto& token : tokens) {
            if (token.startsWith('.')) {
                if (directivesStillAllowed) {
                    directives.insert(token);
                } else {
                    return Error(sourceLine, "Stray '.' in line");
                }
            } else {
                remainingTokens.append(token);
                directivesStillAllowed = false;
            }
        }
        return std::pair<Directives, LineTokens>(directives, remainingTokens);
    }

    virtual QString instrSegment() const { return QStringLiteral(".text"); }
    virtual QString dataSegment() const { return QStringLiteral(".data"); }

    /**
     * @brief m_instructions is the set of instructions which can be matched from an instruction string as well as be
     * disassembled from a program.
     */
    const InstrVec& m_instructions;
    InstrMap m_instructionMap;

    /**
     * @brief m_pseudoInstructions is the set of instructions which can be matched from an instruction string but cannot
     * be disassembled from a program. Typically, pseudoinstructions will expand to one or more non-pseudo instructions.
     */
    const PseudoInstrVec& m_pseudoInstructions;
    PseudoInstrMap m_pseudoInstructionMap;

    /**
     * @brief m_segmentPointers maintains the current end-of-segment pointers for the segments annotated by the program
     * (ie. .text, .data, ...).
     */
    std::map<QString, uint32_t> m_segmentPointers;
    /**
     * @brief m_currentSegment maintains the current segment where the assembler emits information.
     */
    QString m_currentSegment;

    const Matcher<ISA> m_matcher;
};

}  // namespace AssemblerTmp

}  // namespace Ripes
