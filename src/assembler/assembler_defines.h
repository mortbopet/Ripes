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

/// Type for instruction data. Should encompass all possible instruction widths.
using Instr_T = uint64_t;

class Token : public QString {
public:
    inline Token(const QString& t) : QString(t) {}
    inline Token(const QString& t, const QString& relocation) : QString(t), m_relocation(relocation) {}
    inline Token() : QString() {}
    void setRelocation(const QString& relocation) { m_relocation = relocation; }
    bool hasRelocation() const { return !m_relocation.isEmpty(); }
    const QString& relocation() const { return m_relocation; }

private:
    QString m_relocation;
};
using LineTokens = QVector<Token>;
using LineTokensVec = std::vector<LineTokens>;
using Symbols = std::set<Symbol>;
using DirectiveLinePair = std::pair<QString, LineTokens>;
using SymbolMap = std::map<Symbol, VIntS>;
using HandleDirectiveRes = std::variant<Error, std::optional<QByteArray>>;

struct TokenizedSrcLine {
    Symbols symbols;
    LineTokens tokens;
    QString directive;
    unsigned sourceLine = 0;
    AInt programAddress = -1;
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
