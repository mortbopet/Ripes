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

/// Adds a symbol to the current symbol mapping of this assembler defined at the 'line' in the input program.
std::optional<Error> AssemblerBase::addSymbol(const TokenizedSrcLine& line, const Symbol& s, VInt v) const {
    return addSymbol(line.sourceLine, s, v);
}

/// Adds a symbol to the current symbol mapping of this assembler.
std::optional<Error> AssemblerBase::addSymbol(const unsigned& line, const Symbol& s, VInt v) const {
    if (m_symbolMap.count(s)) {
        return {Error(line, "Multiple definitions of symbol '" + s.v + "'")};
    }
    m_symbolMap[s] = v;
    return {};
}

/// Resolves an expression through either the built-in symbol map, or through the expression evaluator.
ExprEvalRes AssemblerBase::evalExpr(const QString& expr) const {
    auto symbolValue = m_symbolMap.find(expr);
    if (symbolValue != m_symbolMap.end()) {
        return symbolValue->second;
    } else {
        return evaluate(expr, &m_symbolMap);
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

std::variant<Error, LineTokens> AssemblerBase::tokenize(const QString& line, const int sourceLine) const {
    auto tokens = line.split(m_splitterRegex);
    tokens.removeAll(QStringLiteral(""));
    auto joinedtokens = joinParentheses(tokens);
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
