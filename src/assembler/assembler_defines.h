#pragma once

#include <QByteArray>
#include <QStringList>

#include <map>
#include <set>
#include <variant>

namespace Ripes {

namespace AssemblerTmp {

using Symbol = QString;
using Symbols = std::set<Symbol>;
using SymbolMap = std::map<Symbol, unsigned>;
using ReverseSymbolMap = std::map<uint32_t, Symbol>;
using LineTokens = QStringList;
struct SourceLine {
    LineTokens tokens;
    unsigned source_line = 0;
    QString symbol;
};
using SymbolLinePair = std::pair<Symbol, LineTokens>;
using Program = std::vector<SourceLine>;
using NoPassResult = std::monostate;

// An error is defined as a reference to a source line index + an error string
using Error = std::pair<unsigned, QString>;
using Errors = std::vector<Error>;
/**
 * @brief The Result struct
 * An assembly result is determined to be valid iff errors is empty.
 */
struct Result {
    Errors errors;
    QByteArray program;
};

}  // namespace AssemblerTmp

}  // namespace Ripes
