#include "assembler.h"

#include <algorithm>

#include <QRegExp>
#include <QRegularExpression>
#include <QString>

namespace {
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

}  // namespace

namespace Ripes {
namespace AssemblerTmp {

Result AssemblerBase::assemble(const QString& program) const {
    const auto programLines = program.split(QRegExp("[\r\n]"));
    Result result;

    runPass(tokenizedLines, Program, pass0, programLines);

    // Pseudo SourceLine expansion
    runPass(unused, Program, pass1, tokenizedLines);

    // Symbol mapping
    Symbols symbols;
    runPass(unused2, NoPassResult, pass2, tokenizedLines, symbols);

    // Machine code translation
    // runPass(unused3, QByteArray, pass3, tokenizedLines, symbols);
}

std::variant<Errors, Program> AssemblerBase::pass0(const QStringList& program) const {
    Errors errors;
    Program tokenizedLines;
    tokenizedLines.reserve(program.size());
    for (unsigned i = 0; i < program.size(); i++) {
        const auto& line = program.at(i);
        if (line.isEmpty())
            continue;
        runOperation(tokenizedLine, LineTokens, tokenize, program[i]);
        tokenizedLines.push_back({tokenizedLine, i, {}});
    }
    if (errors.size() != 0) {
        return {errors};
    } else {
        return {tokenizedLines};
    }
}

std::variant<Errors, Program> AssemblerBase::pass1(const Program& tokenizedLines) const {
    Errors errors;
    Program expandedLines;
    expandedLines.reserve(tokenizedLines.size());

    for (unsigned i = 0; i < tokenizedLines.size(); i++) {
        const auto& tokenizedLine = tokenizedLines.at(i);
        runOperation(expandedOps, std::optional<std::vector<LineTokens>>, expandPseudoOp, tokenizedLine);
        if (expandedOps) {
            for (const auto& expandedOp : expandedOps.value()) {
                /** @note: Original source line is kept for all resulting lines after pseudo-op expantion */
                expandedLines.push_back({expandedOp, tokenizedLine.source_line, {}});
            }
        } else {
            expandedLines.push_back(tokenizedLine);
        }
    }

    if (errors.size() != 0) {
        return {errors};
    } else {
        return {expandedLines};
    }
}

std::variant<Errors, NoPassResult> AssemblerBase::pass2(std::vector<SourceLine>& tokenizedLines,
                                                        Symbols& symbols) const {
    Errors errors;
    for (auto& line : tokenizedLines) {
        runOperation(splitLine, SymbolLinePair, splitSymbolFromLine, line);
        const auto& [symbol, rest] = splitLine;
        if (!symbol.isEmpty()) {
            if (symbols.count(symbol)) {
                errors.push_back({line.source_line, "Symbol '" + symbol + "' is already defined."});
            }
            line.symbol = symbol;
        }
    }

    if (errors.size() != 0) {
        return {errors};
    } else {
        return {NoPassResult()};
    }
}

std::variant<Errors, QByteArray> AssemblerBase::pass3(const Program& tokenizedLines, const SymbolMap& symbolMap) const {
    QByteArray program;
    Errors errors;
    for (const auto& line : tokenizedLines) {
        runOperation(machineCode, QByteArray, assembleInstruction, line, symbolMap);
        program.append(machineCode);
    }
    if (errors.size() != 0) {
        return {errors};
    } else {
        return {program};
    }
}

std::variant<Error, LineTokens> AssemblerBase::tokenize(const QString& line) const {
    const static auto splitter = QRegularExpression(
        R"(\t|\((?=x(?:[1-2]\d|3[0-1]|\d)|t[0-6]|a[0-7]|s(?:1[0-1]|\d)|[sgt]p|zero)|(?:x(?:[1-2]\d|3[0-1]|\d)|t[0-6]|a[0-7]|s(?:1[0-1]|\d)|[sgt]p|zero)\K\))");

    return line.split(splitter);
}

std::variant<Error, std::pair<QString, LineTokens>> AssemblerBase::splitSymbolFromLine(const SourceLine& line) const {
    if (line.tokens.size() == 0) {
        return std::pair<QString, LineTokens>(QString(), line.tokens);
    }

    const bool hasSymbol =
        std::any_of(line.tokens.begin(), line.tokens.end(), [](const auto& s) { return s.contains(':'); });

    const auto& firstToken = line.tokens.at(0);

    if (hasSymbol && !firstToken.contains(':')) {
        return Error(line.source_line, "Line must start with symbol");
    }
    if (firstToken.count(':') > 1) {
        return Error(line.source_line, "Multiple instances of ':' in label");
    }

    // Valid symbol - parse the symbol and remaining tokens
    auto remainingTokens = line.tokens;
    remainingTokens.removeFirst();
    auto symbol = firstToken;
    symbol.remove(':');
    return std::pair<QString, LineTokens>(symbol, remainingTokens);
}

}  // namespace AssemblerTmp
}  // namespace Ripes
