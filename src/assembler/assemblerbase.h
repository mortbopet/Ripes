#pragma once

#include <QRegularExpression>

#include <optional>

#include "assembler_defines.h"
#include "directive.h"
#include "expreval.h"

namespace Ripes {
namespace Assembler {

struct OpDisassembleResult {
    /// Stringified representation of a disassembled machine word.
    QString repr;
    /// Number of bytes disassembled.
    unsigned bytesDisassembled = 0;
    /// Optional error that occured during disassembly.
    std::optional<Error> err;
};

///  Base class for a Ripes assembler.
class AssemblerBase {
public:
    AssemblerBase();
    virtual ~AssemblerBase() {}
    std::optional<Error> setCurrentSegment(Section seg) const;

    /// Sets the base pointer of seg to the provided 'base' value.
    void setSegmentBase(Section seg, AInt base);

    /// Assembles an input program (represented as a list of strings). Optionally, a set of predefined symbols may be
    /// provided to the assemble call.
    /// If programLines does not represent the source program directly (possibly due to conversion of newline/cr/..., an
    /// explicit hash of the source program can be provided for later identification.
    virtual AssembleResult assemble(const QStringList& programLines, const SymbolMap* symbols = nullptr,
                                    QString sourceHash = QString()) const = 0;
    AssembleResult assembleRaw(const QString& program, const SymbolMap* symbols = nullptr) const;

    /// Disassembles an input program relative to the provided base address.
    virtual DisassembleResult disassemble(const Program& program, const AInt baseAddress = 0) const = 0;

    /// Disassembles an input word using the provided symbol mapping relative to the provided base address.
    virtual OpDisassembleResult disassemble(const VInt word, const ReverseSymbolMap& symbols,
                                            const AInt baseAddress = 0) const = 0;

    /// Returns the set of opcodes (as strings) which are supported by this assembler.
    virtual std::set<QString> getOpcodes() const = 0;

    /// Adds a symbol to the current symbol mapping of this assembler defined at the 'line' in the input program.
    std::optional<Error> addSymbol(const TokenizedSrcLine& line, const Symbol& s, VInt v) const;

    /// Adds a symbol to the current symbol mapping of this assembler.
    std::optional<Error> addSymbol(const unsigned& line, const Symbol& s, VInt v) const;

    /// Resolves an expression through either the built-in symbol map, or through the expression evaluator.
    ExprEvalRes evalExpr(const QString& expr) const;

    /// Set the supported directives for this assembler.
    void setDirectives(const DirectiveVec& directives);

protected:
    /// Creates a set of LineTokens by tokenizing a line of source code.
    QRegularExpression m_splitterRegex;
    std::variant<Error, LineTokens> tokenize(const QString& line, const int sourceLine) const;

    HandleDirectiveRes assembleDirective(const DirectiveArg& arg, bool& ok, bool skipEarlyDirectives = true) const;

    /**
     * @brief splitSymbolsFromLine
     * @returns a pair consisting of a symbol and the the input @p line tokens where the symbol has been removed.
     */
    std::variant<Error, SymbolLinePair> splitSymbolsFromLine(const LineTokens& tokens, int sourceLine) const;

    std::variant<Error, DirectiveLinePair> splitDirectivesFromLine(const LineTokens& tokens, int sourceLine) const;

    /// Given an input set of tokens, splits away commented code from the tokens based on the comment delimiter, i.e.:
    /// {"a", "b", "#", "c"} => {"a", "b"}
    std::variant<Error, LineTokens> splitCommentFromLine(const LineTokens& tokens) const;

    /// Returns the comment-delimiting character for this assembler.
    virtual QChar commentDelimiter() const = 0;

    /**
     * @brief m_sectionBasePointers maintains the base position for the segments
     * annoted by the Segment enum class.
     */
    std::map<Section, AInt> m_sectionBasePointers;
    /**
     * @brief m_currentSegment maintains the current segment where the assembler emits information.
     * Marked mutable to allow for switching currently selected segment during assembling.
     */
    mutable Section m_currentSection;

    /**
     * @brief symbolMap maintains the symbols recorded during assembling. Marked mutable to allow for assembler
     * directives to add symbols during assembling.
     */
    mutable SymbolMap m_symbolMap;

    /**
     * The set of supported assembler directives. A assembler can add directives through
     * AssemblerBase::setDirectives.
     */
    DirectiveVec m_directives;
    DirectiveMap m_directivesMap;
    EarlyDirectives m_earlyDirectives;
};

}  // namespace Assembler
}  // namespace Ripes
