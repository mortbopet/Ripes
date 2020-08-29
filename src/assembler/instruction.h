#pragma once

#include "../binutils.h"
#include "assembler_defines.h"

#include <QString>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>
#include "math.h"

namespace Ripes {

namespace AssemblerTmp {

namespace {
using AssembleRes = std::variant<AssemblerTmp::Error, uint32_t>;
using DisassembleRes = std::variant<AssemblerTmp::Error, AssemblerTmp::LineTokens>;
}  // namespace

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

struct Field {
    Field() = default;
    virtual ~Field() = default;
    virtual std::optional<AssemblerTmp::Error> apply(const AssemblerTmp::SourceLine& line,
                                                     uint32_t& instruction) const = 0;
    virtual std::optional<AssemblerTmp::Error> decode(const uint32_t instruction,
                                                      AssemblerTmp::LineTokens& line) const = 0;
};

/** @brief OpPart
 * A segment of an operation-identifying field of an instruction.
 */
struct OpPart {
    OpPart(unsigned _value, BitRange _range) : value(_value), range(_range) {}
    OpPart(unsigned _value, unsigned _start, unsigned _stop) : value(_value), range({_start, _stop}) {}
    constexpr bool matches(uint32_t instruction) const { return range.decode(instruction) == value; }
    const unsigned value;
    const BitRange range;

    bool operator==(const OpPart& other) const { return this->value == other.value && this->range == other.range; }
    bool operator<(const OpPart& other) const { return this->range < other.range || this->value < other.value; }
};

template <typename ISA>
struct Opcode : public Field {
    /**
     * @brief Opcode
     * @param name: Name of operation
     * @param fields: list of OpParts corresponding to the identifying elements of the opcode.
     */
    Opcode(const QString& name, std::vector<OpPart> fields) : m_name(name), m_opParts(fields) {}

    std::optional<AssemblerTmp::Error> apply(const AssemblerTmp::SourceLine&, uint32_t& instruction) const override {
        for (const auto& opPart : m_opParts) {
            instruction |= opPart.range.apply(opPart.value);
        }
    }
    std::optional<AssemblerTmp::Error> decode(const uint32_t instruction,
                                              AssemblerTmp::LineTokens& line) const override {
        line.push_back(m_name);
        return {};
    }

    bool operator==(uint32_t instruction) const {}

    const QString m_name;
    const std::vector<OpPart> m_opParts;
};

template <typename ISA>
struct Reg : public Field {
    /**
     * @brief Reg
     * @param tokenIndex: Index within a list of decoded instruction tokens that corresponds to the register index
     * @param range: range in instruction field containing register index value
     */
    Reg(unsigned tokenIndex, BitRange range) : m_tokenIndex(tokenIndex), m_range(range) {}
    Reg(unsigned tokenIndex, unsigned _start, unsigned _stop) : m_tokenIndex(tokenIndex), m_range({_start, _stop}) {}
    std::optional<AssemblerTmp::Error> apply(const AssemblerTmp::SourceLine& line,
                                             uint32_t& instruction) const override {
        bool success;
        const QString& regToken = line.tokens[m_tokenIndex];
        const uint32_t reg = ISA::instance()->regNumber(regToken, success);
        if (!success) {
            return AssemblerTmp::Error(line.source_line, "Unknown register '" + regToken + "'");
        }
        instruction |= m_range.apply(reg);
    }
    std::optional<AssemblerTmp::Error> decode(const uint32_t instruction,
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
    void apply(const uint32_t& value, uint32_t& instruction) const { instruction |= range.apply(value >> offset); }
    void decode(uint32_t& value, const uint32_t& instruction) const { value |= range.decode(instruction) << offset; };
    const unsigned offset;
    const BitRange range;
};

struct Imm : public Field {
    enum class Repr { Unsigned, Signed, Hex };
    /**
     * @brief Imm
     * @param tokenIndex: Index within a list of decoded instruction tokens that corresponds to the immediate
     * @param ranges: (ordered) list of ranges corresponding to fields of the immediate
     */
    Imm(unsigned _tokenIndex, unsigned _width, Repr _repr, const std::vector<ImmPart>& _parts)
        : tokenIndex(_tokenIndex), parts(_parts), width(_width), repr(_repr) {}

    std::optional<AssemblerTmp::Error> apply(const AssemblerTmp::SourceLine& line,
                                             uint32_t& instruction) const override {
        // @Todo: decode immediate from token (appropriate bit width!), apply each ImmPart with immediate to instruction
    }
    std::optional<AssemblerTmp::Error> decode(const uint32_t instruction,
                                              AssemblerTmp::LineTokens& line) const override {
        uint32_t reconstructed = 0;
        for (const auto& part : parts) {
            part.decode(reconstructed, instruction);
        }
        if (repr == Repr::Signed) {
            reconstructed = signextend<int32_t>(reconstructed, width);
            line.push_back(QString::number(static_cast<int32_t>(reconstructed)));
        } else if (repr == Repr::Unsigned) {
            line.push_back(QString::number(reconstructed));
        } else {
            line.push_back("0x" + QString::number(reconstructed, 16));
        }
        return {};
    }

    const unsigned tokenIndex;
    const std::vector<ImmPart> parts;
    const unsigned width;
    const Repr repr;
};

template <typename ISA>
class Instruction {
public:
    Instruction(Opcode<ISA> opcode, const std::vector<std::shared_ptr<Field>>& fields)
        : m_opcode(opcode), m_expectedTokens(1 /*opcode*/ + fields.size()), m_fields(fields) {
        m_assembler = [this](const AssemblerTmp::SourceLine& line) {
            uint32_t instruction = 0;
            m_opcode.apply(line, instruction);
            for (const auto& field : m_fields)
                field->apply(line, instruction);
            return instruction;
        };
        m_disassembler = [this](uint32_t instruction) {
            AssemblerTmp::LineTokens line;
            m_opcode.decode(instruction, line);
            for (const auto& field : m_fields) {
                if (auto error = field->decode(instruction, line)) {
                    return DisassembleRes(*error);
                }
            }
            return DisassembleRes(line);
        };
    }

    AssembleRes assemble(const AssemblerTmp::SourceLine& line) {
        if (line.tokens.length() != m_expectedTokens) {
            return AssemblerTmp::Error(line.source_line, "Instruction '" + m_opcode.name + "' expects " +
                                                             QString::number(m_expectedTokens - 1) +
                                                             " tokens, but got " +
                                                             QString::number(line.tokens.length() - 1));
        }

        return m_assembler(line);
    }
    DisassembleRes disassemble(uint32_t instruction) const { return m_disassembler(instruction); }

    const Opcode<ISA>& getOpcode() const { return m_opcode; }
    const QString& name() const { return m_opcode.m_name; }

private:
    std::function<AssembleRes(const AssemblerTmp::SourceLine&)> m_assembler;
    std::function<DisassembleRes(uint32_t)> m_disassembler;

    const Opcode<ISA> m_opcode;
    const int m_expectedTokens;
    const std::vector<std::shared_ptr<Field>> m_fields;
};

}  // namespace AssemblerTmp

}  // namespace Ripes
