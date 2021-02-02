#pragma once

#include <QRegularExpression>

#include "assembler_defines.h"
#include "directive.h"
#include "expreval.h"
#include "instruction.h"
#include "isa/isainfo.h"
#include "matcher.h"
#include "parserutilities.h"
#include "pseudoinstruction.h"

#include <set>
#include <variant>

namespace Ripes {
namespace Assembler {

/**
 * A macro for running an assembler pass with error handling
 */
#define runPass(resName, resType, passFunction, ...)                               \
    auto passFunction##_res = passFunction(__VA_ARGS__);                           \
    if (auto* errors = std::get_if<Errors>(&passFunction##_res)) {                 \
        result.errors.insert(result.errors.end(), errors->begin(), errors->end()); \
        return result;                                                             \
    }                                                                              \
    auto resName = std::get<resType>(passFunction##_res);

/**
 * A macro for running an assembler operation which may throw an error or return a value (expressed through a variant
 * type) which runs inside a loop. In case of the operation throwing an error, the error is recorded and the next
 * iteration of the loop is executed. If not, the result value is provided through variable 'resName'.
 */
#define runOperation(resName, resType, operationFunction, ...)        \
    auto operationFunction##_res = operationFunction(__VA_ARGS__);    \
    if (auto* error = std::get_if<Error>(&operationFunction##_res)) { \
        errors.push_back(*error);                                     \
        continue;                                                     \
    }                                                                 \
    auto resName = std::get<resType>(operationFunction##_res);

class AssemblerBase {
public:
    std::optional<Error> setCurrentSegment(Section seg) const {
        if (m_sectionBasePointers.count(seg) == 0) {
            return Error(0, "No base address set for segment '" + seg + +"'");
        }
        m_currentSection = seg;
        return {};
    }

    void setSegmentBase(Section seg, uint32_t base) {
        if (m_sectionBasePointers.count(seg) != 0) {
            throw std::runtime_error("Base address already set for segment '" + seg.toStdString() + +"'");
        }
        m_sectionBasePointers[seg] = base;
    }

    virtual AssembleResult assemble(const QStringList& programLines) const = 0;
    AssembleResult assembleRaw(const QString& program) const {
        const auto programLines = program.split(QRegExp("[\r\n]"));
        return assemble(programLines);
    }

    virtual DisassembleResult disassemble(const Program& program, const uint32_t baseAddress = 0) const = 0;
    virtual std::pair<QString, std::optional<Error>> disassemble(const uint32_t word, const ReverseSymbolMap& symbols,
                                                                 const uint32_t baseAddress = 0) const = 0;

    virtual std::set<QString> getOpcodes() const = 0;

protected:
    void setDirectives(DirectiveVec& directives) {
        if (m_directives.size() != 0) {
            throw std::runtime_error("Directives already set");
        }
        m_directives = directives;
        for (const auto& iter : m_directives) {
            const auto directive = iter.get()->name();
            if (m_directivesMap.count(directive) != 0) {
                throw std::runtime_error("Error: directive " + directive.toStdString() +
                                         " has already been registerred.");
            }
            m_directivesMap[directive] = iter;
        }
    }

    virtual std::variant<Error, LineTokens> tokenize(const QString& line, const int sourceLine) const {
        // Regex: Split on all empty strings (\s+) and characters [, \[, \], \(, \)] except for quote-delimitered
        // substrings.
        const static auto splitter = QRegularExpression(R"((\s+|\,)(?=(?:[^"]*"[^"]*")*[^"]*$))");
        auto tokens = line.split(splitter);
        tokens.removeAll(QStringLiteral(""));
        auto joinedtokens = joinParentheses(tokens);
        if (auto* err = std::get_if<Error>(&joinedtokens)) {
            err->first = sourceLine;
            return *err;
        }
        return std::get<LineTokens>(joinedtokens);
    }

    virtual HandleDirectiveRes assembleDirective(const TokenizedSrcLine& line, bool& ok) const {
        ok = false;
        if (line.directive.isEmpty()) {
            return std::nullopt;
        }
        ok = true;
        try {
            return m_directivesMap.at(line.directive)->handle(this, line);
        } catch (const std::out_of_range&) {
            return {Error(line.sourceLine, "Unknown directive '" + line.directive + "'")};
        }
    };
    /**
     * @brief splitSymbolsFromLine
     * @returns a pair consisting of a symbol and the the input @p line tokens where the symbol has been removed.
     */
    virtual std::variant<Error, SymbolLinePair> splitSymbolsFromLine(const LineTokens& tokens, int sourceLine) const {
        if (tokens.size() == 0) {
            return {SymbolLinePair({}, tokens)};
        }

        // Handle the case where a token has been joined together with another token, ie "B:nop"
        QStringList splitTokens;
        splitTokens.reserve(tokens.size());
        for (const auto& token : tokens) {
            QString buffer;
            for (const auto& ch : token) {
                buffer.append(ch);
                if (ch == ':') {
                    splitTokens << buffer;
                    buffer.clear();
                }
            }
            if (!buffer.isEmpty()) {
                splitTokens << buffer;
            }
        }

        LineTokens remainingTokens;
        remainingTokens.reserve(splitTokens.size());
        Symbols symbols;
        bool symbolStillAllowed = true;
        for (const auto& token : splitTokens) {
            if (token.endsWith(':')) {
                if (symbolStillAllowed) {
                    const QString cleanedSymbol = token.left(token.length() - 1);
                    if (symbols.count(cleanedSymbol) != 0) {
                        return {Error(sourceLine, "Multiple definitions of symbol '" + cleanedSymbol + "'")};
                    } else {
                        if (cleanedSymbol.isEmpty() || cleanedSymbol.contains(s_exprOperatorsRegex)) {
                            return {Error(sourceLine, "Invalid symbol '" + cleanedSymbol + "'")};
                        }

                        symbols.insert(cleanedSymbol);
                    }
                } else {
                    return {Error(sourceLine, QStringLiteral("Stray ':' in line"))};
                }
            } else {
                remainingTokens.append(token);
                symbolStillAllowed = false;
            }
        }
        return {SymbolLinePair(symbols, remainingTokens)};
    }

    virtual std::variant<Error, DirectiveLinePair> splitDirectivesFromLine(const LineTokens& tokens,
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
                remainingTokens.append(token);
                directivesStillAllowed = false;
            }
        }
        if (directives.size() > 1) {
            return {Error(sourceLine, QStringLiteral("Illegal multiple directives"))};
        } else {
            return {DirectiveLinePair(directives.size() == 1 ? directives[0] : QString(), remainingTokens)};
        }
    }

    virtual std::variant<Error, LineTokens> splitCommentFromLine(const LineTokens& tokens) const {
        if (tokens.size() == 0) {
            return {tokens};
        }

        LineTokens preCommentTokens;
        preCommentTokens.reserve(tokens.size());
        for (const auto& token : tokens) {
            if (token.contains(commentDelimiter())) {
                break;
            } else {
                preCommentTokens.append(token);
            }
        }
        return {preCommentTokens};
    }

    virtual QChar commentDelimiter() const = 0;

    /**
     * @brief m_assemblerDirectives is the set of supported assembler directives.
     */
    DirectiveVec m_directives;
    DirectiveMap m_directivesMap;

    /**
     * @brief m_sectionBasePointers maintains the base position for the segments
     * annoted by the Segment enum class.
     */
    std::map<Section, uint32_t> m_sectionBasePointers;
    /**
     * @brief m_currentSegment maintains the current segment where the assembler emits information.
     * Marked mutable to allow for switching currently selected segment during assembling.
     */
    mutable Section m_currentSection;
};

class Assembler : public AssemblerBase {
public:
    Assembler(const ISAInfoBase* isa) : m_isa(isa) {}

    AssembleResult assemble(const QStringList& programLines) const override {
        AssembleResult result;

        // Per default, emit to .text until otherwise specified
        setCurrentSegment(".text");

        // Tokenize each source line and separate symbol from remainder of tokens
        runPass(tokenizedLines, SourceProgram, pass0, programLines);

        // Pseudo instruction expansion
        runPass(expandedLines, SourceProgram, pass1, tokenizedLines);

        /** Assemble. During assembly, we generate:
         * - symbolMap: Recoding the offset locations in the program of lines adorned with symbols
         * - linkageMap: Recording offsets of instructions which require linkage with symbols
         */
        SymbolMap symbolMap;
        LinkRequests needsLinkage;
        runPass(program, Program, pass2, expandedLines, symbolMap, needsLinkage);

        // Symbol linkage
        runPass(unused, NoPassResult, pass3, program, symbolMap, needsLinkage);

        result.program = program;
        return result;
    }

    DisassembleResult disassemble(const Program& program, const uint32_t baseAddress = 0) const override {
        size_t i = 0;
        DisassembleResult res;
        auto& programBits = program.getSection(".text")->data;
        while ((i + sizeof(uint32_t)) <= programBits.size()) {
            const uint32_t instructionWord = *reinterpret_cast<const uint32_t*>(programBits.data() + i);
            auto disres = disassemble(instructionWord, program.symbols, baseAddress + i);
            if (disres.second) {
                res.errors.push_back(disres.second.value());
            }
            res.program << disres.first;
            i += sizeof(uint32_t);
        }
        return res;
    }

    std::pair<QString, std::optional<Error>> disassemble(const uint32_t word, const ReverseSymbolMap& symbols,
                                                         const uint32_t baseAddress = 0) const override {
        auto match = m_matcher->matchInstruction(word);
        if (auto* error = std::get_if<Error>(&match)) {
            return {"unknown instruction", *error};
        }

        // Got match, disassemble
        auto tokens = std::get<const Instruction*>(match)->disassemble(word, baseAddress, symbols);
        if (auto* error = std::get_if<Error>(&match)) {
            // Error during disassembling
            return {"invalid instruction", *error};
        }

        return {std::get<LineTokens>(tokens).join(' '), {}};
    }

    const Matcher& getMatcher() { return *m_matcher; }

    std::set<QString> getOpcodes() const override {
        std::set<QString> opcodes;
        for (const auto& iter : m_instructionMap) {
            opcodes.insert(iter.first);
        }
        for (const auto& iter : m_pseudoInstructionMap) {
            opcodes.insert(iter.first);
        }
        return opcodes;
    }

protected:
    struct LinkRequest {
        unsigned sourceLine;  // Source location of code which resulted in the link request
        uint32_t offset;      // Offset of instruction in segment which needs link resolution
        Section section;      // Section which instruction was emitted in

        // Reference to the immediate field which resolves the symbol and the requested symbol
        FieldLinkRequest fieldRequest;
    };

    uint32_t linkReqAddress(const LinkRequest& req) const { return req.offset + m_sectionBasePointers.at(req.section); }

    using LinkRequests = std::vector<LinkRequest>;

    /**
     * @brief pass0
     * Line tokenization and source line recording
     */
    std::variant<Errors, SourceProgram> pass0(const QStringList& program) const {
        Errors errors;
        SourceProgram tokenizedLines;
        tokenizedLines.reserve(program.size());
        Symbols symbols;

        /** @brief carry
         * A symbol should refer to the next following assembler line; whether an instruction or directive.
         * The carry is used to carry over symbol definitions from empty lines onto the next valid line.
         * Multiple symbol definitions is checked in this step given that a symbol may loose its source line
         * information if it is a blank symbol (no other information on line).
         */
        Symbols carry;
        for (int i = 0; i < program.size(); i++) {
            const auto& line = program.at(i);
            if (line.isEmpty())
                continue;
            TokenizedSrcLine tsl;
            tsl.sourceLine = i;
            runOperation(tokens, LineTokens, tokenize, program[i], i);

            runOperation(remainingTokens, LineTokens, splitCommentFromLine, tokens);

            // Symbols precede directives
            runOperation(symbolsAndRest, SymbolLinePair, splitSymbolsFromLine, remainingTokens, i);
            tsl.symbols = symbolsAndRest.first;

            bool uniqueSymbols = true;
            for (const auto& s : symbolsAndRest.first) {
                if (symbols.count(s) != 0) {
                    errors.push_back(Error(i, "Multiple definitions of symbol '" + s + "'"));
                    uniqueSymbols = false;
                    break;
                }
            }
            if (!uniqueSymbols) {
                continue;
            }
            symbols.insert(symbolsAndRest.first.begin(), symbolsAndRest.first.end());

            runOperation(directiveAndRest, DirectiveLinePair, splitDirectivesFromLine, symbolsAndRest.second, i);
            tsl.directive = directiveAndRest.first;

            tsl.tokens = directiveAndRest.second;
            if (tsl.tokens.empty() && tsl.directive.isEmpty()) {
                if (!tsl.symbols.empty()) {
                    carry.insert(tsl.symbols.begin(), tsl.symbols.end());
                }
            } else {
                tsl.symbols.insert(carry.begin(), carry.end());
                carry.clear();
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
    std::variant<Errors, SourceProgram> pass1(const SourceProgram& tokenizedLines) const {
        Errors errors;
        SourceProgram expandedLines;
        expandedLines.reserve(tokenizedLines.size());

        for (unsigned i = 0; i < tokenizedLines.size(); i++) {
            const auto& tokenizedLine = tokenizedLines.at(i);
            runOperation(expandedOps, std::optional<std::vector<LineTokens>>, expandPseudoOp, tokenizedLine);
            if (expandedOps) {
                /** @note: Original source line is kept for all resulting lines after pseudo-op expantion.
                 * Labels and directives are only kept for the first expanded op.
                 */
                const auto& eops = expandedOps.value();
                for (size_t j = 0; j < eops.size(); j++) {
                    TokenizedSrcLine tsl;
                    tsl.tokens = eops.at(j);
                    tsl.sourceLine = tokenizedLine.sourceLine;
                    if (j == 0) {
                        tsl.directive = tokenizedLine.directive;
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
     * Machine code translation. If @return errors is empty, pass succeeded.
     * In the following, current size of the program is used as an analog for the offset of the to-be-assembled
     * instruction in the program. This is then used for symbol resolution.
     */
    std::variant<Errors, Program> pass2(const SourceProgram& tokenizedLines, SymbolMap& symbolMap,
                                        LinkRequests& needsLinkage) const {
        // Initialize program with initialized segments:
        Program program;
        for (const auto& iter : m_sectionBasePointers) {
            ProgramSection sec;
            sec.name = iter.first;
            sec.address = iter.second;
            sec.data = QByteArray();
            program.sections[iter.first] = sec;
        }

        Errors errors;
        QByteArray* currentSection = &program.sections.at(m_currentSection).data;

        bool wasDirective;
        for (const auto& line : tokenizedLines) {
            // Get offset of currently emitting position in memory relative to section position
            uint32_t addr_offset = currentSection->size();
            for (const auto& s : line.symbols) {
                // Record symbol position as its absolute address in memory
                // No check on duplicate symbols, done in pass 1
                symbolMap[s] = addr_offset + program.sections.at(m_currentSection).address;
            }

            runOperation(directiveBytes, std::optional<QByteArray>, assembleDirective, line, wasDirective);

            // Currently emitting segment may have changed during the assembler directive; refresh state
            currentSection = &program.sections.at(m_currentSection).data;
            addr_offset = currentSection->size();
            if (!wasDirective) {
                std::weak_ptr<Instruction> assembledWith;
                runOperation(machineCode, InstrRes, assembleInstruction, line, assembledWith);

                if (!machineCode.linksWithSymbol.symbol.isEmpty()) {
                    LinkRequest req;
                    req.sourceLine = line.sourceLine;
                    req.offset = addr_offset;
                    req.fieldRequest = machineCode.linksWithSymbol;
                    req.section = m_currentSection;
                    needsLinkage.push_back(req);
                }
                currentSection->append(
                    QByteArray(reinterpret_cast<char*>(&machineCode.instruction), sizeof(machineCode.instruction)));

            }
            // This was a directive; check if any bytes needs to be appended to the segment
            else if (directiveBytes) {
                currentSection->append(directiveBytes.value());
            }
        }
        if (errors.size() != 0) {
            return {errors};
        }

        // Register symbols in program struct
        for (const auto& iter : symbolMap) {
            program.symbols[iter.second] = iter.first;
        }

        return {program};
    }

    std::variant<Errors, NoPassResult> pass3(Program& program, SymbolMap& symbolMap,
                                             const LinkRequests& needsLinkage) const {
        Errors errors;
        for (const auto& linkRequest : needsLinkage) {
            const auto& symbol = linkRequest.fieldRequest.symbol;
            uint32_t symbolValue;

            // Add the special __address__ symbol indicating the address of the instruction itself
            symbolMap["__address__"] = linkReqAddress(linkRequest);

            if (symbolMap.count(symbol) == 0) {
                if (couldBeExpression(symbol)) {
                    // No recorded symbol for the token; our last option is to try and evaluate a possible
                    // expression.
                    auto evaluate_res = evaluate(symbol, &symbolMap);
                    if (auto* err = std::get_if<Error>(&evaluate_res)) {
                        err->first = linkRequest.sourceLine;
                        errors.push_back(*err);
                        continue;
                    }
                    // Expression evaluated successfully
                    symbolValue = std::get<long>(evaluate_res);
                } else {
                    errors.push_back(Error(linkRequest.sourceLine, "Unknown symbol '" + symbol + "'"));
                    continue;
                }
            } else {
                symbolValue = symbolMap.at(symbol);
            }

            QByteArray& section = program.sections.at(linkRequest.section).data;

            // Decode instruction at link-request position
            assert(section.size() >= (linkRequest.offset + 4) &&
                   "Error: position of link request is not within program");
            uint32_t instr = *reinterpret_cast<uint32_t*>(section.data() + linkRequest.offset);

            // Re-apply immediate resolution using the value acquired from the symbol map
            if (auto* immField = dynamic_cast<const Imm*>(linkRequest.fieldRequest.field)) {
                immField->applySymbolResolution(symbolValue, instr, linkReqAddress(linkRequest));
            } else {
                assert(false && "Something other than an immediate field has requested linkage?");
            }

            // Finally, overwrite the instruction in the section
            *reinterpret_cast<uint32_t*>(section.data() + linkRequest.offset) = instr;
        }
        if (errors.size() != 0) {
            return {errors};
        } else {
            return {NoPassResult()};
        }
    }

    virtual PseudoExpandRes expandPseudoOp(const TokenizedSrcLine& line) const {
        if (line.tokens.empty()) {
            return PseudoExpandRes(std::nullopt);
        }
        const auto& opcode = line.tokens.at(0);
        if (m_pseudoInstructionMap.count(opcode) == 0) {
            // Not a pseudo instruction
            return PseudoExpandRes(std::nullopt);
        }
        auto res = m_pseudoInstructionMap.at(opcode)->expand(line);
        if (auto* error = std::get_if<Error>(&res)) {
            if (m_instructionMap.count(opcode) != 0) {
                // If this pseudo-instruction aliases with an instruction but threw an error (could arise if ie.
                // arguments provided were intended for the normal instruction and not the pseudoinstruction), then
                // return as if not a pseudo-instruction, falling to normal instruction handling
                return PseudoExpandRes(std::nullopt);
            }
        }

        // Return result (containing either a valid pseudo-instruction expand error or the expanded pseudo
        // instruction
        return {res};
    }

    virtual AssembleRes assembleInstruction(const TokenizedSrcLine& line,
                                            std::weak_ptr<Instruction>& assembledWith) const {
        if (line.tokens.empty()) {
            return {Error(line.sourceLine, "Empty source lines should be impossible at this point")};
        }
        const auto& opcode = line.tokens.at(0);
        if (m_instructionMap.count(opcode) == 0) {
            return {Error(line.sourceLine, "Unknown opcode '" + opcode + "'")};
        }
        assembledWith = m_instructionMap.at(opcode);
        return m_instructionMap.at(opcode)->assemble(line);
    };

    void initialize(InstrVec& instructions, PseudoInstrVec& pseudoinstructions, DirectiveVec& directives) {
        setInstructions(instructions);
        setPseudoInstructions(pseudoinstructions);
        setDirectives(directives);
        m_matcher = std::make_unique<Matcher>(m_instructions);
    }

    void setPseudoInstructions(PseudoInstrVec& pseudoInstructions) {
        if (m_pseudoInstructions.size() != 0) {
            throw std::runtime_error("Pseudoinstructions already set");
        }
        m_pseudoInstructions = pseudoInstructions;

        for (const auto& iter : m_pseudoInstructions) {
            const auto instr_name = iter.get()->name();
            if (m_pseudoInstructionMap.count(instr_name) != 0) {
                throw std::runtime_error("Error: pseudo-instruction with opcode '" + instr_name.toStdString() +
                                         "' has already been registerred.");
            }
            m_pseudoInstructionMap[instr_name] = iter;
        }
    }

    void setInstructions(InstrVec& instructions) {
        if (m_instructions.size() != 0) {
            throw std::runtime_error("Instructions already set");
        }
        m_instructions = instructions;
        for (const auto& iter : m_instructions) {
            const auto instr_name = iter.get()->name();
            if (m_instructionMap.count(instr_name) != 0) {
                throw std::runtime_error("Error: instruction with opcode '" + instr_name.toStdString() +
                                         "' has already been registerred.");
            }
            m_instructionMap[instr_name] = iter;
        }
    }

    /**
     * @brief m_instructions is the set of instructions which can be matched from an instruction string as well as
     * be disassembled from a program.
     */
    InstrVec m_instructions;
    InstrMap m_instructionMap;

    /**
     * @brief m_pseudoInstructions is the set of instructions which can be matched from an instruction string but
     * cannot be disassembled from a program. Typically, pseudoinstructions will expand to one or more non-pseudo
     * instructions.
     */
    PseudoInstrVec m_pseudoInstructions;
    PseudoInstrMap m_pseudoInstructionMap;

    std::unique_ptr<Matcher> m_matcher;

    const ISAInfoBase* m_isa;
};

}  // namespace Assembler

}  // namespace Ripes
