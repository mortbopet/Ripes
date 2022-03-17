#include "assemblerbase.h"

#include "parserutilities.h"

namespace Ripes {
namespace Assembler {

AssemblerBase::AssemblerBase() {
    /// Set the source line splitter regex and immediately JIT compile it.
    /// Regex: Split on all empty strings (\s+) and characters [, \[, \], \(, \)] except for quote-delimitered
    /// substrings.
    m_splitterRegex = QRegularExpression(R"((\s+|\,)(?=(?:[^"]*"[^"]*")*[^"]*$))");
    m_splitterRegex.optimize();
}

std::optional<Error> AssemblerBase::setCurrentSegment(Section seg) const {
    if (m_sectionBasePointers.count(seg) == 0) {
        return Error(0, "No base address set for segment '" + seg + +"'");
    }
    m_currentSection = seg;
    return {};
}

/// Sets the base pointer of seg to the provided 'base' value.
void AssemblerBase::setSegmentBase(Section seg, AInt base) {
    m_sectionBasePointers[seg] = base;
}

AssembleResult AssemblerBase::assembleRaw(const QString& program, const SymbolMap* symbols) const {
    const auto programLines = program.split(QRegExp("[\r\n]"));
    return assemble(programLines, symbols, Program::calculateHash(program.toUtf8()));
}

/// Resolves an expression through either the built-in symbol map, or through the expression evaluator.
ExprEvalRes AssemblerBase::evalExpr(const QString& expr, unsigned sourceLine) const {
    auto relativeMap = m_symbolMap.copyRelativeTo(sourceLine);

    auto symbolValue = relativeMap.find(expr);
    if (symbolValue != relativeMap.end()) {
        return symbolValue->second;
    } else {
        return evaluate(expr, &relativeMap);
    }
}

void AssemblerBase::setDirectives(const DirectiveVec& directives) {
    if (m_directives.size() != 0) {
        throw std::runtime_error("Directives already set");
    }
    m_directives = directives;
    for (const auto& iter : m_directives) {
        const auto directive = iter.get()->name();
        if (m_directivesMap.count(directive) != 0) {
            throw std::runtime_error("Error: directive " + directive.toStdString() + " has already been registerred.");
        }
        m_directivesMap[directive] = iter;
        if (iter->early()) {
            m_earlyDirectives.insert(iter->name());
        }
    }
}

std::variant<Error, LineTokens> AssemblerBase::tokenize(const QString& line, int sourceLine) const {
    auto quoteTokenized = tokenizeQuotes(line, sourceLine);
    if (auto* error = std::get_if<Error>(&quoteTokenized))
        return {*error};

    auto joinedtokens = joinParentheses(std::get<QStringList>(quoteTokenized));
    if (auto* err = std::get_if<Error>(&joinedtokens)) {
        err->first = sourceLine;
        return *err;
    }
    return std::get<LineTokens>(joinedtokens);
}

HandleDirectiveRes AssemblerBase::assembleDirective(const DirectiveArg& arg, bool& ok, bool skipEarlyDirectives) const {
    ok = false;
    if (arg.line.directive.isEmpty()) {
        return std::nullopt;
    }
    ok = true;
    try {
        const auto& directive = m_directivesMap.at(arg.line.directive);
        if (directive->early() && skipEarlyDirectives) {
            return std::nullopt;
        }
        return directive->handle(this, arg);
    } catch (const std::out_of_range&) {
        return {Error(arg.line.sourceLine, "Unknown directive '" + arg.line.directive + "'")};
    }
}

/**
 * @brief splitSymbolsFromLine
 * @returns a pair consisting of a symbol and the the input @p line tokens where the symbol has been removed.
 */
std::variant<Error, SymbolLinePair> AssemblerBase::splitSymbolsFromLine(const LineTokens& tokens,
                                                                        int sourceLine) const {
    if (tokens.size() == 0) {
        return {SymbolLinePair({}, tokens)};
    }

    // Handle the case where a token has been joined together with another token, ie "B:nop"
    LineTokens splitTokens;
    splitTokens.reserve(tokens.size());
    for (const auto& token : tokens) {
        if (token.startsWith('\"') && token.endsWith('\"')) {
            // Skip quoted strings.
            splitTokens.push_back(token);
            continue;
        }
        Token buffer;
        for (const auto& ch : token) {
            buffer.append(ch);
            if (ch == ':') {
                splitTokens.push_back(buffer);
                buffer.clear();
            }
        }
        if (!buffer.isEmpty()) {
            splitTokens.push_back(buffer);
        }
    }

    LineTokens remainingTokens;
    remainingTokens.reserve(splitTokens.size());
    Symbols symbols;
    bool symbolStillAllowed = true;
    for (const auto& token : splitTokens) {
        if (token.endsWith(':')) {
            if (symbolStillAllowed) {
                const Symbol cleanedSymbol = Symbol(token.left(token.length() - 1), Symbol::Type::Address);
                if (symbols.count(cleanedSymbol.v) != 0) {
                    return {Error(sourceLine, "Multiple definitions of symbol '" + cleanedSymbol.v + "'")};
                } else {
                    if (cleanedSymbol.v.isEmpty() || cleanedSymbol.v.contains(s_exprOperatorsRegex)) {
                        return {Error(sourceLine, "Invalid symbol '" + cleanedSymbol.v + "'")};
                    }

                    symbols.insert(cleanedSymbol);
                }
            } else {
                return {Error(sourceLine, QStringLiteral("Stray ':' in line"))};
            }
        } else {
            remainingTokens.push_back(token);
            symbolStillAllowed = false;
        }
    }
    return {SymbolLinePair(symbols, remainingTokens)};
}

std::variant<Error, DirectiveLinePair> AssemblerBase::splitDirectivesFromLine(const LineTokens& tokens,
                                                                              int sourceLine) const {
    if (tokens.size() == 0) {
        return {DirectiveLinePair(QString(), tokens)};
    }

    LineTokens remainingTokens;
    remainingTokens.reserve(tokens.size());
    std::vector<QString> directives;
    bool directivesStillAllowed = true;
    for (const auto& token : tokens) {
        if (token.startsWith('.')) {
            if (directivesStillAllowed) {
                directives.push_back(token);
            } else {
                return {Error(sourceLine, QStringLiteral("Stray '.' in line"))};
            }
        } else {
            remainingTokens.push_back(token);
            directivesStillAllowed = false;
        }
    }
    if (directives.size() > 1) {
        return {Error(sourceLine, QStringLiteral("Illegal multiple directives"))};
    } else {
        return {DirectiveLinePair(directives.size() == 1 ? directives[0] : QString(), remainingTokens)};
    }
}

std::variant<Error, LineTokens> AssemblerBase::splitCommentFromLine(const LineTokens& tokens) const {
    if (tokens.size() == 0) {
        return {tokens};
    }

    LineTokens preCommentTokens;
    preCommentTokens.reserve(tokens.size());
    for (const auto& token : tokens) {
        if (token.contains(commentDelimiter())) {
            break;
        } else {
            preCommentTokens.push_back(token);
        }
    }
    return {preCommentTokens};
}

}  // namespace Assembler
}  // namespace Ripes
