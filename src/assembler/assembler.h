#pragma once

#include <QRegularExpression>

#include <cstdint>
#include <numeric>
#include <set>
#include <variant>

#include "STLExtras.h"
#include "assembler_defines.h"
#include "assemblerbase.h"
#include "gnudirectives.h"
#include "isa/instruction.h"
#include "isa/isa_defines.h"
#include "isa/isa_types.h"
#include "isa/isainfo.h"
#include "isa/pseudoinstruction.h"
#include "matcher.h"
#include "ripessettings.h"

namespace Ripes {
namespace Assembler {

/**
 * A macro for running an assembler pass with error handling
 */
#define runPass(resName, resType, passFunction, ...)                           \
  auto passFunction##_res = passFunction(__VA_ARGS__);                         \
  if (auto *errors = std::get_if<Errors>(&passFunction##_res)) {               \
    result.errors.insert(result.errors.end(), errors->begin(), errors->end()); \
    assert(result.errors.size() != 0);                                         \
    return result;                                                             \
  }                                                                            \
  auto resName = std::get<resType>(passFunction##_res);

/**
 * A macro for running an assembler operation which may throw an error or return
 * a value (expressed through a variant type) which runs inside a loop. In case
 * of the operation throwing an error, the error is recorded and the next
 * iteration of the loop is executed. If not, the result value is provided
 * through variable 'resName'.
 */
#define runOperation(resName, operationFunction, ...)                          \
  auto operationFunction##_res = operationFunction(__VA_ARGS__);               \
  if (operationFunction##_res.isError()) {                                     \
    errors.push_back(operationFunction##_res.error());                         \
    continue;                                                                  \
  }                                                                            \
  auto resName = operationFunction##_res.value();

#define runOperationNoRes(operationFunction, ...)                              \
  auto operationFunction##_res = operationFunction(__VA_ARGS__);               \
  if (operationFunction##_res) {                                               \
    errors.push_back(operationFunction##_res.value());                         \
    assert(errors.size() != 0);                                                \
    continue;                                                                  \
  }

/**
 *  Reg_T: type equal in size to the register width of the target
 *  Instr_T: type equal in size to the instruction width of the target
 */
class Assembler : public AssemblerBase {
  static_assert(std::numeric_limits<Reg_T>::is_integer,
                "Register type must be integer");

public:
  explicit Assembler(std::shared_ptr<const ISAInfoBase> isa) : m_isa(isa) {}

  AssembleResult
  assemble(const QStringList &programLines, const SymbolMap *symbols = nullptr,
           const QString &sourceHash = QString()) const override {
    AssembleResult result;

    /// by default, emit to .text until otherwise specified
    setCurrentSegment(Location::unknown(), ".text");
    m_symbolMap.clear();
    if (symbols) {
      m_symbolMap = *symbols;
    }

    /// Tokenize each source line and separate symbol from remainder of tokens
    runPass(tokenizedLines, SourceProgram, pass0, programLines);

    /// Pseudo instruction expansion
    runPass(expandedLines, SourceProgram, pass1, tokenizedLines);

    /** Assemble. During assembly, we generate:
     * - linkageMap: Recording offsets of instructions which require linkage
     * with symbols
     */
    LinkRequests needsLinkage;
    runPass(program, Program, pass2, expandedLines, needsLinkage);

    // Symbol linkage
    runPass(unused, NoPassResult, pass3, program, needsLinkage);
    Q_UNUSED(unused);

    result.program = program;
    result.program.sourceHash = sourceHash;
    result.program.entryPoint = m_sectionBasePointers.at(".text");
    return result;
  }

  DisassembleResult disassemble(const Program &program,
                                const AInt baseAddress = 0) const override {
    VInt progByteIter = 0;
    DisassembleResult res;
    auto &programBits = program.getSection(".text")->data;
    bool cont = true;
    while (cont) {
      const Instr_T instructionWord =
          *reinterpret_cast<const Instr_T *>(programBits.data() + progByteIter);
      auto disres = disassemble(instructionWord, program.symbols,
                                baseAddress + progByteIter);
      res.program << disres.repr;
      if (disres.err.has_value()) {
        res.errors.push_back(disres.err.value());
        /// Default to 4-byte increments.
        /// @todo: this should rather be "to the next aligned boundary", with
        /// alignment beying defined by the ISA.
        progByteIter += 4;
      } else {
        /// Increment byte iterator by # of bytes disassembled. This is needed
        /// for ISAs with variable instruction width.
        progByteIter += disres.bytesDisassembled;
      }
      cont = progByteIter <= static_cast<VInt>(programBits.size());
    }
    return res;
  }

  OpDisassembleResult disassemble(const VInt word,
                                  const ReverseSymbolMap &symbols,
                                  const AInt baseAddress = 0) const override {
    OpDisassembleResult opres;

    auto match = m_matcher->matchInstruction(word);
    if (auto *error = std::get_if<Error>(&match)) {
      opres.repr = "Unknown instruction";
      opres.err = *error;
      return opres;
    }

    // Got match, disassemble
    auto instruction = std::get<const InstructionBase *>(match);
    auto tokensVar = instruction->disassemble(word, baseAddress, symbols);
    if (auto *error = std::get_if<Error>(&tokensVar)) {
      // Error during disassembling
      opres.repr = "Invalid instruction";
      opres.err = *error;
      return opres;
    }

    // Join tokens
    QString joinedLine;
    const auto tokens = std::get<LineTokens>(tokensVar);
    for (const auto &token : std::as_const(tokens)) {
      joinedLine += token + " ";
    }
    joinedLine.chop(1); // remove trailing ' '
    opres.repr = joinedLine;
    opres.bytesDisassembled = instruction->size();
    return opres;
  }

  const Matcher &getMatcher() { return *m_matcher; }

  std::set<QString> getOpcodes() const override {
    std::set<QString> opcodes;
    for (const auto &iter : m_instructionMap) {
      opcodes.insert(iter.first);
    }
    for (const auto &iter : m_pseudoInstructionMap) {
      opcodes.insert(iter.first);
    }
    return opcodes;
  }

  const InstrVec &getInstructionSet() const override {
    return m_isa->instructions();
  }

  const PseudoInstrVec &getPseudoInstructionSet() const override {
    return m_isa->pseudoInstructions();
  }

  void setRelocations() {
    if (m_relocationsMap.size() != 0) {
      throw std::runtime_error("Directives already set");
    }
    for (const auto &iter : m_isa->relocations()) {
      const auto relocation = iter.get()->name();
      if (m_relocationsMap.count(relocation) != 0) {
        throw std::runtime_error("Error: relocation " +
                                 relocation.toStdString() +
                                 " has already been registerred.");
      }
      m_relocationsMap[relocation] = iter;
    }
  }

protected:
  struct LinkRequest : public Location {
    // sourceLine: Source location of code which resulted in the link request
    LinkRequest(const Location &location) : Location(location) {}
    Reg_T
        offset; // Offset of instruction in segment which needs link resolution
    Section section;         // Section which instruction was emitted in
    unsigned instrAlignment; // Alignment of instruction in bytes

    // Reference to the immediate field which resolves the symbol and the
    // requested symbol
    FieldLinkRequest fieldRequest;
  };

  Reg_T linkReqAddress(const LinkRequest &req) const {
    return req.offset + m_sectionBasePointers.at(req.section);
  }

  using LinkRequests = std::vector<LinkRequest>;

  /**
   * @brief pass0
   * Line tokenization and source line recording
   */
  std::variant<Errors, SourceProgram> pass0(const QStringList &program) const {
    Errors errors;
    SourceProgram tokenizedLines;
    tokenizedLines.reserve(program.size());
    Symbols symbols;

    /** @brief carry
     * A symbol should refer to the next following assembler line; whether an
     * instruction or directive. The carry is used to carry over symbol
     * definitions from empty lines onto the next valid line. Multiple symbol
     * definitions is checked in this step given that a symbol may loose its
     * source line information if it is a blank symbol (no other information on
     * line).
     */
    Symbols carry;
    for (auto line : llvm::enumerate(program)) {
      if (line.value().isEmpty())
        continue;
      TokenizedSrcLine tsl(line.index());
      runOperation(tokens, tokenize, tsl, line.value());

      runOperation(remainingTokens, splitCommentFromLine, tokens);

      // Symbols precede directives
      runOperation(symbolsAndRest, splitSymbolsFromLine, tsl, remainingTokens);

      tsl.symbols = symbolsAndRest.first;

      bool uniqueSymbols = true;
      for (const auto &s : symbolsAndRest.first) {
        if (!s.isLegal())
          errors.push_back(Error(tsl, "Illegal symbol '" + s.v + "'"));

        if (!s.isLocal() && symbols.count(s) != 0) {
          errors.push_back(
              Error(tsl, "Multiple definitions of symbol '" + s.v + "'"));
          uniqueSymbols = false;
          break;
        }
      }
      if (!uniqueSymbols) {
        continue;
      }
      symbols.insert(symbolsAndRest.first.begin(), symbolsAndRest.first.end());

      runOperation(directiveAndRest, splitDirectivesFromLine, tsl,
                   symbolsAndRest.second);
      tsl.directive = directiveAndRest.first;

      // Parse (and remove) relocation hints from the tokens.
      runOperation(finalTokens, splitRelocationsFromLine,
                   directiveAndRest.second);

      tsl.tokens = finalTokens;
      if (tsl.tokens.empty() && tsl.directive.isEmpty()) {
        if (!tsl.symbols.empty()) {
          carry.insert(tsl.symbols.begin(), tsl.symbols.end());
        }
      } else {
        tsl.symbols.insert(carry.begin(), carry.end());
        carry.clear();
        tokenizedLines.push_back(tsl);
      }

      if (!tsl.directive.isEmpty() && m_earlyDirectives.count(tsl.directive)) {
        bool wasDirective; // unused
        runOperation(directiveBytes, assembleDirective,
                     DirectiveArg{tsl, nullptr}, wasDirective, false);
      }
    }

    if (!errors.empty()) {
      return {errors};
    } else {
      return {tokenizedLines};
    }
  }

  /**
   * @brief pass1
   * Pseudo-op expansion. If @return errors is empty, pass succeeded.
   */
  std::variant<Errors, SourceProgram>
  pass1(const SourceProgram &tokenizedLines) const {
    Errors errors;
    SourceProgram expandedLines;
    expandedLines.reserve(tokenizedLines.size());

    for (auto tokenizedLine : llvm::enumerate(tokenizedLines)) {
      auto expandedOps = expandPseudoOp(tokenizedLine.value());
      if (expandedOps.isResult()) {
        /** @note: Original source line is kept for all resulting lines after
         * pseudo-op expantion. Labels and directives are only kept for the
         * first expanded op.
         */
        const auto &eops = expandedOps.value();
        for (auto eop : llvm::enumerate(eops)) {
          TokenizedSrcLine tsl(tokenizedLine.value().sourceLine());
          tsl.tokens = eop.value();
          if (eop.index() == 0) {
            tsl.directive = tokenizedLine.value().directive;
            tsl.symbols = tokenizedLine.value().symbols;
          }
          expandedLines.push_back(tsl);
        }
      } else {
        // This was not a pseudoinstruction; just add line to the set of
        // expanded lines
        expandedLines.push_back(tokenizedLine.value());
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
   * Machine code translation. If @return errors is empty, pass succeeded.
   * In the following, current size of the program is used as an analog for the
   * offset of the to-be-assembled instruction in the program. This is then used
   * for symbol resolution.
   */
  std::variant<Errors, Program> pass2(const SourceProgram &tokenizedLines,
                                      LinkRequests &needsLinkage) const {
    // Initialize program with initialized segments:
    Program program;
    for (const auto &iter : m_sectionBasePointers) {
      ProgramSection sec;
      sec.name = iter.first;
      sec.address = iter.second;
      sec.data = QByteArray();
      program.sections[iter.first] = sec;
    }

    Errors errors;
    ProgramSection *currentSection = &program.sections.at(m_currentSection);

    bool wasDirective;
    for (const auto &line : tokenizedLines) {
      // Get offset of currently emitting position in memory relative to section
      // position
      VInt addr_offset = currentSection->data.size();
      for (const auto &s : line.symbols) {
        // Record symbol position as its absolute address in memory
        auto res = m_symbolMap.addSymbol(
            line, s,
            addr_offset + program.sections.at(m_currentSection).address);
        if (res) {
          errors.push_back(res.value());
          continue;
        }
      }

      runOperation(directiveBytes, assembleDirective,
                   DirectiveArg{line, currentSection}, wasDirective);

      // Currently emitting segment may have changed during the assembler
      // directive; refresh state
      currentSection = &program.sections.at(m_currentSection);
      addr_offset = currentSection->data.size();
      if (!wasDirective) {
        /// Maintain a pointer to the instruction that was assembled.
        std::shared_ptr<InstructionBase> assembledWith;
        runOperation(machineCode, assembleInstruction, line, assembledWith);
        assert(assembledWith && "Expected the assembler instruction to be set");
        program.sourceMapping[addr_offset].insert(line.sourceLine());

        if (!machineCode.linksWithSymbol.symbol.isEmpty()) {
          LinkRequest req(line.sourceLine());
          req.offset = addr_offset;
          req.fieldRequest = machineCode.linksWithSymbol;
          req.section = m_currentSection;
          req.instrAlignment = m_isa->instrByteAlignment();
          needsLinkage.push_back(req);
        }

        /// Check if we're now misaligned wrt. the size of the instruction.
        /// Instructions should always be emitted on an aligned boundary wrt.
        /// their size.
        const unsigned alignmentDiff =
            addr_offset % (m_isa->instrByteAlignment());
        if (alignmentDiff != 0) {
          errors.push_back(
              {line.sourceLine(),
               "Instruction misaligned (" + QString::number(alignmentDiff * 8) +
                   "-bit boundary). This instruction must be aligned on a " +
                   QString::number((m_isa->instrByteAlignment()) * 8) +
                   "-bit boundary."});
          break;
        }

        currentSection->data.append(
            QByteArray(reinterpret_cast<char *>(&machineCode.instruction),
                       assembledWith->size()));
      }
      // This was a directive; append any assembled bytes to the segment.
      currentSection->data.append(directiveBytes);
    }
    if (errors.size() != 0) {
      return {errors};
    }

    // Register address symbols in program struct.
    /// @todo: also consider relative symbols here.
    for (const auto &iter : m_symbolMap.abs) {
      if (iter.first.is(Symbol::Type::Address)) {
        program.symbols[iter.second] = iter.first;
      }
    }

    return {program};
  }

  std::variant<Errors, NoPassResult>
  pass3(Program &program, const LinkRequests &needsLinkage) const {
    Errors errors;
    for (const LinkRequest &linkRequest : needsLinkage) {
      const auto &symbol = linkRequest.fieldRequest.symbol;
      Reg_T symbolValue;

      // Add the special __address__ symbol indicating the address of the
      // instruction itself. Not done through addSymbol given that we redefine
      // this symbol on each line.
      const Reg_T linkRequestAddress = linkReqAddress(linkRequest);
      m_symbolMap.abs["__address__"] = linkRequestAddress;

      // Expression evaluation also performs symbol evaluation
      auto exprRes = evalExpr(linkRequest, symbol);
      if (auto *err = std::get_if<Error>(&exprRes)) {
        errors.push_back(*err);
        continue;
      } else {
        symbolValue = std::get<ExprEvalVT>(exprRes);
      }

      if (!linkRequest.fieldRequest.relocation.isEmpty()) {
        auto relocRes = m_relocationsMap.at(linkRequest.fieldRequest.relocation)
                            .get()
                            ->handle(symbolValue, linkRequestAddress);
        if (auto *error = std::get_if<Error>(&relocRes)) {
          errors.push_back(*error);
          continue;
        }
        symbolValue = std::get<Reg_T>(relocRes);
      }

      QByteArray &section = program.sections.at(linkRequest.section).data;

      // Decode instruction at link-request position
      assert(static_cast<unsigned>(section.size()) >=
                 (linkRequest.offset + linkRequest.instrAlignment) &&
             "Error: position of link request is not within program");
      Instr_T instr =
          *reinterpret_cast<Instr_T *>(section.data() + linkRequest.offset);

      // Re-apply immediate resolution using the value acquired from the symbol
      // map
      assert(linkRequest.fieldRequest.resolveSymbol &&
             "Something other than an immediate field has requested linkage?");
      if (auto res = linkRequest.fieldRequest.resolveSymbol(
              linkRequest, symbolValue, instr, linkReqAddress(linkRequest));
          res.isError()) {
        errors.push_back(res.error());
        continue;
      }

      // Finally, overwrite the instruction in the section
      *reinterpret_cast<Instr_T *>(section.data() + linkRequest.offset) = instr;
    }
    if (errors.size() != 0) {
      return {errors};
    } else {
      return {NoPassResult()};
    }
  }

  virtual Result<std::vector<LineTokens>>
  expandPseudoOp(const TokenizedSrcLine &line) const {
    if (line.tokens.empty()) {
      return Error(line, "Not a pseudo instruction");
    }
    const auto &opcode = line.tokens.at(0);
    if (m_pseudoInstructionMap.count(opcode) == 0) {
      // Not a pseudo instruction
      return Error(line, "Not a pseudo instruction");
    }
    auto res = m_pseudoInstructionMap.at(opcode)->expand(line, m_symbolMap);
    if (auto *error = std::get_if<Error>(&res)) {
      Q_UNUSED(error);
      if (m_instructionMap.count(opcode) != 0) {
        // If this pseudo-instruction aliases with an instruction but
        // threw an error (could arise if ie. arguments provided were
        // intended for the normal instruction and not the
        // pseudoinstruction), then return as if not a pseudo-instruction,
        // falling to normal instruction handling
        return Error(line, "");
      }
    }

    // Return result (containing either a valid pseudo-instruction expand error
    // or the expanded pseudo instruction
    return {res};
  }

  virtual AssembleRes
  assembleInstruction(const TokenizedSrcLine &line,
                      std::shared_ptr<InstructionBase> &assembledWith) const {
    if (line.tokens.empty()) {
      return {
          Error(line, "Empty source lines should be impossible at this point")};
    }
    const auto &opcode = line.tokens.at(0);
    auto instrIt = m_instructionMap.find(opcode);
    if (instrIt == m_instructionMap.end()) {
      return {Error(line, "Unknown opcode '" + opcode + "'")};
    }
    assembledWith = instrIt->second;
    return assembledWith->assemble(line);
  }

  void setPseudoInstructions() {
    if (m_pseudoInstructionMap.size() != 0) {
      throw std::runtime_error("Pseudoinstructions already set");
    }
    for (const auto &iter : m_isa->pseudoInstructions()) {
      const auto instr_name = iter.get()->name();
      if (m_pseudoInstructionMap.count(instr_name) != 0) {
        throw std::runtime_error("Error: pseudo-instruction with opcode '" +
                                 instr_name.toStdString() +
                                 "' has already been registerred.");
      }
      m_pseudoInstructionMap[instr_name] = iter;
    }
  }

  void setInstructions() {
    if (m_instructionMap.size() != 0) {
      throw std::runtime_error("Instructions already set");
    }
    for (const auto &iter : m_isa->instructions()) {
      const auto instr_name = iter.get()->name();
      if (m_instructionMap.count(instr_name) != 0) {
        throw std::runtime_error("Error: instruction with opcode '" +
                                 instr_name.toStdString() +
                                 "' has already been registerred.");
      }
      m_instructionMap[instr_name] = iter;
    }
  }

  /**
   * @brief splitRelocationsFromLine
   * Identify relocations in tokens; when found, add the relocation to the
   * _following_ token. Relocations are _not_ added to the set of remaining
   * tokens.
   */
  virtual Result<LineTokens>
  splitRelocationsFromLine(LineTokens &tokens) const {
    if (tokens.size() == 0) {
      return {tokens};
    }

    LineTokens remainingTokens;
    remainingTokens.reserve(tokens.size());

    Token relocationForNextToken;
    for (auto &token : tokens) {
      if (m_relocationsMap.count(token)) {
        relocationForNextToken = token;
      } else {
        if (!relocationForNextToken.isEmpty()) {
          token.setRelocation(relocationForNextToken);
          relocationForNextToken.clear();
        }
        remainingTokens.push_back(token);
      }
    }

    return {remainingTokens};
  }

  void initialize(const DirectiveVec &directives) {
    setInstructions();
    setPseudoInstructions();
    setDirectives(directives);
    setRelocations();
    m_matcher = std::make_unique<Matcher>(m_isa->instructions());
  }

  /**
   * @brief m_instructionMap contains the set of instructions which can be
   * matched from an instruction string as well as be disassembled from a
   * program.
   */
  InstrMap m_instructionMap;

  /**
   * @brief m_pseudoInstructionMap contains the set of instructions which can be
   * matched from an instruction string but cannot be disassembled from a
   * program. Typically, pseudoinstructions will expand to one or more
   * non-pseudo instructions.
   */
  PseudoInstrMap m_pseudoInstructionMap;

  /**
   * @brief m_relocationMap contains the set of supported assembler relocation
   * hints
   */
  RelocationsMap m_relocationsMap;

  std::unique_ptr<Matcher> m_matcher;

  std::shared_ptr<const ISAInfoBase> m_isa;
};

/// An Assembler and QObject (workaround because QObject cannot be directly
/// subclassed by ISA_Assembler)
class QAssembler : public QObject, public Assembler {
  Q_OBJECT
public:
  QAssembler(std::shared_ptr<const ISAInfoBase> isaInfo) : Assembler(isaInfo) {
    // Initialize segment pointers and monitor settings changes to segment
    // pointers
    connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_TEXTSTART),
            &SettingObserver::modified, this, [this](const QVariant &value) {
              setSegmentBase(".text", value.toULongLong());
            });
    RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_TEXTSTART)->trigger();
    connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_DATASTART),
            &SettingObserver::modified, this, [this](const QVariant &value) {
              setSegmentBase(".data", value.toULongLong());
            });
    RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_DATASTART)->trigger();
    connect(RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_BSSSTART),
            &SettingObserver::modified, this, [this](const QVariant &value) {
              setSegmentBase(".bss", value.toULongLong());
            });
    RipesSettings::getObserver(RIPES_SETTING_ASSEMBLER_BSSSTART)->trigger();
  }
};

/// An ISA-specific assembler
template <ISA isa>
struct ISA_Assembler : public QAssembler {
  ISA_Assembler(std::shared_ptr<const ISAInfo<isa>> isaInfo)
      : QAssembler(isaInfo) {
    initialize(gnuDirectives());
  }

  ISA getISA() const override { return isa; }

protected:
  QChar commentDelimiter() const override { return '#'; }
};

/// Returns an assembler for isa
/// Throws a runtime error if the isa does not have a matching assembler
std::shared_ptr<AssemblerBase>
constructAssemblerDynamic(const std::shared_ptr<const ISAInfoBase> &isa);

} // namespace Assembler

} // namespace Ripes
