#pragma once

#include "../binutils.h"
#include "assembler_defines.h"

#include <QString>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>
#include "binutils.h"
#include "isainfo.h"
#include "math.h"

namespace Ripes {

namespace AssemblerTmp {

struct BitRange {
    constexpr BitRange(unsigned _start, unsigned _stop, unsigned _N = 32) : start(_start), stop(_stop), N(_N) {
        assert(isPowerOf2(_N) && "Bitrange N must be power of 2");
        assert(0 <= _start && _start <= _stop && _stop < _N && "invalid range");
    }
    constexpr unsigned width() const { return stop - start + 1; }
    const unsigned start, stop, N;
    const uint32_t mask = generateBitmask(width());

    uint32_t apply(uint32_t value) const { return (value & mask) << start; }
    uint32_t decode(uint32_t instruction) const { return (instruction >> start) & mask; };

    bool operator==(const BitRange& other) const { return this->start == other.start && this->stop == other.stop; }
    bool operator<(const BitRange& other) const { return this->start < other.start; }
};

struct Field;
struct FieldLinkRequest {
    Field const* field = nullptr;
    QString symbol = QString();
};

struct Field {
    Field() = default;
    virtual ~Field() = default;
    virtual std::optional<AssemblerTmp::Error> apply(const AssemblerTmp::TokenizedSrcLine& line, uint32_t& instruction,
                                                     FieldLinkRequest& linksWithSymbol) const = 0;
    virtual std::optional<AssemblerTmp::Error> decode(const uint32_t instruction, const uint32_t address,
                                                      const ReverseSymbolMap& symbolMap,
                                                      AssemblerTmp::LineTokens& line) const = 0;
};

/**
 * @brief The InstrRes struct is returned by any assembler. Contains the assembled instruction alongside a flag noting
 * whether the instruction needs additional linkage (ie. for symbol resolution).
 */
struct InstrRes {
    uint32_t instruction = 0;
    FieldLinkRequest linksWithSymbol;
};

using AssembleRes = std::variant<Error, InstrRes>;
using PseudoExpandRes = std::variant<Error, std::optional<std::vector<AssemblerTmp::LineTokens>>>;
using DisassembleRes = std::variant<Error, AssemblerTmp::LineTokens>;

/** @brief OpPart
 * A segment of an operation-identifying field of an instruction.
 */
struct OpPart {
    OpPart(unsigned _value, BitRange _range) : value(_value), range(_range) {}
    OpPart(unsigned _value, unsigned _start, unsigned _stop) : value(_value), range({_start, _stop}) {}
    const bool matches(uint32_t instruction) const { return range.decode(instruction) == value; }
    const unsigned value;
    const BitRange range;

    bool operator==(const OpPart& other) const { return this->value == other.value && this->range == other.range; }
    bool operator<(const OpPart& other) const { return this->range < other.range || this->value < other.value; }
};

struct Opcode : public Field {
    /**
     * @brief Opcode
     * @param name: Name of operation
     * @param fields: list of OpParts corresponding to the identifying elements of the opcode.
     */
    Opcode(const QString& _name, std::vector<OpPart> _opParts) : name(_name), opParts(_opParts) {}

    std::optional<AssemblerTmp::Error> apply(const AssemblerTmp::TokenizedSrcLine&, uint32_t& instruction,
                                             FieldLinkRequest&) const override {
        for (const auto& opPart : opParts) {
            instruction |= opPart.range.apply(opPart.value);
        }
    }
    std::optional<AssemblerTmp::Error> decode(const uint32_t, const uint32_t, const ReverseSymbolMap&,
                                              AssemblerTmp::LineTokens& line) const override {
        line.push_back(name);
        return {};
    }

    bool operator==(uint32_t instruction) const {}

    const QString name;
    const std::vector<OpPart> opParts;
};

template <typename ISA>
struct Reg : public Field {
    static_assert(std::is_base_of<ISAInfoBase, ISA>::value, "Provided ISA type must derive from ISAInfoBase");
    /**
     * @brief Reg
     * @param tokenIndex: Index within a list of decoded instruction tokens that corresponds to the register index
     * @param range: range in instruction field containing register index value
     */
    Reg(unsigned tokenIndex, BitRange range) : m_tokenIndex(tokenIndex), m_range(range) {}
    Reg(unsigned tokenIndex, unsigned _start, unsigned _stop) : m_tokenIndex(tokenIndex), m_range({_start, _stop}) {}
    std::optional<AssemblerTmp::Error> apply(const AssemblerTmp::TokenizedSrcLine& line, uint32_t& instruction,
                                             FieldLinkRequest&) const override {
        bool success;
        const QString& regToken = line.tokens[m_tokenIndex];
        const uint32_t reg = ISA::instance()->regNumber(regToken, success);
        if (!success) {
            return AssemblerTmp::Error(line.sourceLine, "Unknown register '" + regToken + "'");
        }
        instruction |= m_range.apply(reg);
    }
    std::optional<AssemblerTmp::Error> decode(const uint32_t instruction, const uint32_t address,
                                              const ReverseSymbolMap& symbolMap,
                                              AssemblerTmp::LineTokens& line) const override {
        const unsigned regNumber = m_range.decode(instruction);
        const QString registerName = ISA::instance()->regName(regNumber);
        if (registerName.isEmpty()) {
            return AssemblerTmp::Error(0, "Unknown register number '" + QString::number(regNumber) + "'");
        }
        line.append(registerName);
        return {};
    }

    const unsigned m_tokenIndex;
    const BitRange m_range;
};

struct ImmPart {
    ImmPart(unsigned _offset, BitRange _range) : offset(_offset), range(_range) {}
    ImmPart(unsigned _offset, unsigned _start, unsigned _stop) : offset(_offset), range({_start, _stop}) {}
    void apply(const uint32_t value, uint32_t& instruction) const { instruction |= range.apply(value >> offset); }
    void decode(uint32_t& value, const uint32_t instruction) const { value |= range.decode(instruction) << offset; };
    const unsigned offset;
    const BitRange range;
};

struct Imm : public Field {
    enum class Repr { Unsigned, Signed, Hex };
    enum class SymbolType { None, Relative, Absolute };
    /**
     * @brief Imm
     * @param tokenIndex: Index within a list of decoded instruction tokens that corresponds to the immediate
     * @param ranges: (ordered) list of ranges corresponding to fields of the immediate
     * @param symbolType: Set if this immediate refers to a relative or absolute symbol.
     */
    Imm(unsigned _tokenIndex, unsigned _width, Repr _repr, const std::vector<ImmPart>& _parts,
        SymbolType _symbolType = SymbolType::None)
        : tokenIndex(_tokenIndex), parts(_parts), width(_width), repr(_repr), symbolType(_symbolType) {}

    std::optional<AssemblerTmp::Error> apply(const AssemblerTmp::TokenizedSrcLine& line, uint32_t& instruction,
                                             FieldLinkRequest& linksWithSymbol) const override {
        bool success;
        const QString& immToken = line.tokens[tokenIndex];

        // Accept base 10, 16 and 2
        uint32_t uvalue;
        int32_t svalue;
        if (repr == Repr::Signed) {
            svalue = immToken.toInt(&success, 10);
        } else {
            uvalue = immToken.toUInt(&success, 10);
        }
        if (!success && immToken.toUpper().startsWith(QStringLiteral("0X"))) {
            uvalue = immToken.toUInt(&success, 16);
        }
        if (!success && immToken.toUpper().startsWith(QStringLiteral("0B"))) {
            uvalue = immToken.toUInt(&success, 2);
        }

        if (!success) {
            // Could not directly resolve immediate. Register it as a symbol to link to.
            linksWithSymbol.field = this;
            linksWithSymbol.symbol = immToken;
            return {};
        }

        if (!((repr == Repr::Signed && isInt(width, svalue)) || (isUInt(width, uvalue)))) {
            return AssemblerTmp::Error(line.sourceLine, "Immediate value '" + immToken + "' does not fit in " +
                                                            QString::number(width) + " bits");
        }

        for (const auto& part : parts) {
            part.apply(repr == Repr::Signed ? static_cast<uint32_t>(svalue) : uvalue, instruction);
        }
        return {};
    }

    std::optional<AssemblerTmp::Error> applySymbolResolution(uint32_t symbolValue, uint32_t& instruction,
                                                             const uint32_t address) const {
        long adjustedValue = symbolValue;
        if (symbolType == SymbolType::Relative) {
            adjustedValue -= address;
        }

        for (const auto& part : parts) {
            part.apply(adjustedValue, instruction);
        }
        return {};
    }

    std::optional<AssemblerTmp::Error> decode(const uint32_t instruction, const uint32_t address,
                                              const ReverseSymbolMap& symbolMap,
                                              AssemblerTmp::LineTokens& line) const override {
        uint32_t reconstructed = 0;
        for (const auto& part : parts) {
            part.decode(reconstructed, instruction);
        }
        if (repr == Repr::Signed) {
            line.push_back(QString::number(static_cast<int32_t>(signextend<int32_t>(reconstructed, width))));
        } else if (repr == Repr::Unsigned) {
            line.push_back(QString::number(reconstructed));
        } else {
            line.push_back("0x" + QString::number(reconstructed, 16));
        }

        if (symbolType != SymbolType::None) {
            const int value = signextend<int32_t>(reconstructed, width);
            const uint32_t symbolAddress = value + (symbolType == SymbolType::Absolute ? 0 : address);
            if (symbolMap.count(symbolAddress)) {
                line.push_back("<" + symbolMap.at(symbolAddress) + ">");
            }
        }

        return {};
    }

    const unsigned tokenIndex;
    const std::vector<ImmPart> parts;
    const unsigned width;
    const Repr repr;
    const SymbolType symbolType;
};

template <typename ISA>
class Instruction {
    static_assert(std::is_base_of<ISAInfoBase, ISA>::value, "Provided ISA type must derive from ISAInfoBase");

public:
    Instruction(Opcode opcode, const std::vector<std::shared_ptr<Field>>& fields)
        : m_opcode(opcode), m_expectedTokens(1 /*opcode*/ + fields.size()), m_fields(fields) {
        m_assembler = [](const Instruction& _this, const AssemblerTmp::TokenizedSrcLine& line) {
            InstrRes res;
            _this.m_opcode.apply(line, res.instruction, res.linksWithSymbol);
            for (const auto& field : _this.m_fields) {
                auto err = field->apply(line, res.instruction, res.linksWithSymbol);
                if (err) {
                    return AssembleRes(err.value());
                }
            }
            return AssembleRes(res);
        };
        m_disassembler = [](const Instruction& _this, const uint32_t instruction, const uint32_t address,
                            const ReverseSymbolMap& symbolMap) {
            AssemblerTmp::LineTokens line;
            _this.m_opcode.decode(instruction, address, symbolMap, line);
            for (const auto& field : _this.m_fields) {
                if (auto error = field->decode(instruction, address, symbolMap, line)) {
                    return DisassembleRes(*error);
                }
            }
            return DisassembleRes(line);
        };
    }

    AssembleRes assemble(const AssemblerTmp::TokenizedSrcLine& line) const {
        if (line.tokens.length() != m_expectedTokens) {
            return AssemblerTmp::Error(line.sourceLine, "Instruction '" + m_opcode.name + "' expects " +
                                                            QString::number(m_expectedTokens - 1) +
                                                            " tokens, but got " +
                                                            QString::number(line.tokens.length() - 1));
        }
        return m_assembler(*this, line);
    }
    DisassembleRes disassemble(const uint32_t instruction, const uint32_t address,
                               const ReverseSymbolMap& symbolMap) const {
        return m_disassembler(*this, instruction, address, symbolMap);
    }

    const Opcode& getOpcode() const { return m_opcode; }
    const QString& name() const { return m_opcode.name; }
    /**
     * @brief size
     * @return size of assembled instruction, in byte
     */
    const unsigned& size() const { return 4; }

private:
    std::function<AssembleRes(const Instruction&, const AssemblerTmp::TokenizedSrcLine&)> m_assembler;
    std::function<DisassembleRes(const Instruction&, const uint32_t, const uint32_t, const ReverseSymbolMap&)>
        m_disassembler;

    const Opcode m_opcode;
    const int m_expectedTokens;
    const std::vector<std::shared_ptr<Field>> m_fields;
};

template <typename ISA>
class PseudoInstruction {
    static_assert(std::is_base_of<ISAInfoBase, ISA>::value, "Provided ISA type must derive from ISAInfoBase");

public:
    PseudoInstruction(
        const QString& opcode, const std::vector<std::shared_ptr<Field>>& fields,
        const std::function<PseudoExpandRes(const PseudoInstruction&, const AssemblerTmp::TokenizedSrcLine&)>& expander)
        : m_opcode(opcode), m_expectedTokens(1 /*opcode*/ + fields.size()), m_fields(fields), m_expander(expander) {}

    PseudoExpandRes expand(const AssemblerTmp::TokenizedSrcLine& line) {
        if (line.tokens.length() != m_expectedTokens) {
            return AssemblerTmp::Error(
                line.sourceLine, "Instruction '" + m_opcode + "' expects " + QString::number(m_expectedTokens - 1) +
                                     " tokens, but got " + QString::number(line.tokens.length() - 1));
        }

        return m_expander(*this, line);
    }

    const QString& name() const { return m_opcode; }

    /**
     * @brief reg() and imm() return dummy register and immediate expected tokens for a pseudo instruction.
     * Does not relate to actual regs/imms in an instruction word, but solely used for a pseudoinstruction to
     * be used within the instruction token checking system.
     */
    static std::shared_ptr<Reg<ISA>> reg() { return std::make_shared<Reg<ISA>>(0, 0, 0); }
    static std::shared_ptr<Imm> imm() { return std::make_shared<Imm>(0, 0, Imm::Repr::Hex, std::vector<ImmPart>{}); }

private:
    std::function<PseudoExpandRes(const PseudoInstruction& /*this*/, const AssemblerTmp::TokenizedSrcLine&)> m_expander;

    const QString m_opcode;
    const int m_expectedTokens;
    const std::vector<std::shared_ptr<Field>> m_fields;
};

}  // namespace AssemblerTmp

}  // namespace Ripes
