#pragma once

#include <QRegularExpression>

#include <optional>

#include "assembler_defines.h"
#include "directive.h"
#include "expreval.h"
#include "isa/instruction.h"
#include "isa/isainfo.h"
#include "isa/pseudoinstruction.h"
#include "isa/symbolmap.h"

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
  std::optional<Error> setCurrentSegment(const Location &location,
                                         const Section &seg) const;

  /// Sets the base pointer of seg to the provided 'base' value.
  void setSegmentBase(Section seg, AInt base);

  /// Returns the ISA that this assembler is used for.
  virtual ISA getISA() const = 0;

  /// Assembles an input program (represented as a list of strings). Optionally,
  /// a set of predefined symbols may be provided to the assemble call. If
  /// programLines does not represent the source program directly (possibly due
  /// to conversion of newline/cr/..., an explicit hash of the source program
  /// can be provided for later identification.
  virtual AssembleResult
  assemble(const QStringList &programLines, const SymbolMap *symbols = nullptr,
           const QString &sourceHash = QString()) const = 0;
  AssembleResult assembleRaw(const QString &program,
                             const SymbolMap *symbols = nullptr) const;

  /// Disassembles an input program relative to the provided base address.
  virtual DisassembleResult disassemble(const Program &program,
                                        const AInt baseAddress = 0) const = 0;

  /// Disassembles an input word using the provided symbol mapping relative to
  /// the provided base address.
  virtual OpDisassembleResult disassemble(const VInt word,
                                          const ReverseSymbolMap &symbols,
                                          const AInt baseAddress = 0) const = 0;

  /// Returns the set of opcodes (as strings) which are supported by this
  /// assembler.
  virtual std::set<QString> getOpcodes() const = 0;

  /// Returns the map of instructions supported by this assembler.
  virtual const InstrVec &getInstructionSet() const = 0;

  /// Returns the map of pseudo-instructions supported by this assembler.
  virtual const PseudoInstrVec &getPseudoInstructionSet() const = 0;

  /// Resolves an expression through either the built-in symbol map, or through
  /// the expression evaluator.
  ExprEvalRes evalExpr(const Location &location, const QString &expr) const;

  /// Set the supported directives for this assembler.
  void setDirectives(const DirectiveVec &directives);

  /**
   * @brief symbolMap maintains the symbols recorded during assembling. Marked
   * mutable to allow for assembler directives to add symbols during assembling.
   * publically exposed to allow for Directives (such as equDirective) to be
   * able to insert symbols into the assembler... this might be smelly code
   * (!!!).
   */
  mutable SymbolMap m_symbolMap;

protected:
  /// Creates a set of LineTokens by tokenizing a line of source code.
  QRegularExpression m_splitterRegex;
  Result<LineTokens> tokenize(const Location &location,
                              const QString &line) const;

  Result<QByteArray> assembleDirective(const DirectiveArg &arg, bool &ok,
                                       bool skipEarlyDirectives = true) const;

  /**
   * @brief splitSymbolsFromLine
   * @returns a pair consisting of a symbol and the the input @p line tokens
   * where the symbol has been removed.
   */
  Result<SymbolLinePair> splitSymbolsFromLine(const Location &location,
                                              const LineTokens &tokens) const;

  Result<DirectiveLinePair>
  splitDirectivesFromLine(const Location &location,
                          const LineTokens &tokens) const;

  /// Given an input set of tokens, splits away commented code from the tokens
  /// based on the comment delimiter, i.e.:
  /// {"a", "b", "#", "c"} => {"a", "b"}
  Result<LineTokens> splitCommentFromLine(const LineTokens &tokens) const;

  /// Returns the comment-delimiting character for this assembler.
  virtual QChar commentDelimiter() const = 0;

  /**
   * @brief m_sectionBasePointers maintains the base position for the segments
   * annoted by the Segment enum class.
   */
  std::map<Section, AInt> m_sectionBasePointers;
  /**
   * @brief m_currentSegment maintains the current segment where the assembler
   * emits information. Marked mutable to allow for switching currently selected
   * segment during assembling.
   */
  mutable Section m_currentSection;

  /**
   * The set of supported assembler directives. A assembler can add directives
   * through AssemblerBase::setDirectives.
   */
  DirectiveVec m_directives;
  DirectiveMap m_directivesMap;
  EarlyDirectives m_earlyDirectives;
};

} // namespace Assembler
} // namespace Ripes
