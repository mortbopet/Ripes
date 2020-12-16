#pragma once

#include <QByteArray>
#include <QStringList>

#include <map>
#include <optional>
#include <set>
#include <variant>

#include "assemblererror.h"
#include "program.h"

namespace Ripes {
namespace Assembler {

using LineTokens = QStringList;
using LineTokensVec = std::vector<LineTokens>;
using Symbol = QString;
using Symbols = std::set<Symbol>;
using DirectiveLinePair = std::pair<QString, LineTokens>;
using SymbolMap = std::map<Symbol, uint32_t>;
using ReverseSymbolMap = std::map<uint32_t, Symbol>;
using HandleDirectiveRes = std::variant<Error, std::optional<QByteArray>>;
struct TokenizedSrcLine {
    Symbols symbols;
    LineTokens tokens;
    QString directive;
    unsigned sourceLine = 0;
    uint32_t programAddress = -1;
};
using SymbolLinePair = std::pair<Symbols, LineTokens>;
using SourceProgram = std::vector<TokenizedSrcLine>;
using NoPassResult = std::monostate;

using Section = QString;

/**
 * @brief The Result struct
 * An assembly result is determined to be valid iff errors is empty.
 */
struct AssembleResult {
    Errors errors;
    Program program;
};

struct DisassembleResult {
    Errors errors;
    QStringList program;
};

}  // namespace Assembler

}  // namespace Ripes
