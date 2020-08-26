#pragma once

#include "../binutils.h"
#include "assembler.h"

#include <QString>
#include <map>
#include <type_traits>
#include <vector>
#include "math.h"

namespace Ripes {

namespace {
using AssembleRes = std::variant<Assembler::Error, uint32_t>;
using DisassembleRes = std::variant<Assembler::Error, Assembler::LineTokens>;
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
};

class Field {
public:
    Field(const std::vector<unsigned>& _tokenIndexes, const std::vector<BitRange>& _ranges)
        : ranges(_ranges), tokenIndexes(_tokenIndexes) {
        assertRangesMutuallyExclusive();
    }
    virtual ~Field() = default;
    virtual std::optional<Assembler::Error> apply(const Assembler::SourceLine& line, uint32_t& instruction) const = 0;
    virtual std::optional<Assembler::Error> decode(const uint32_t instruction, Assembler::LineTokens& line) const = 0;

protected:
    const std::vector<BitRange> ranges;
    const std::vector<unsigned> tokenIndexes;

private:
    /**
     * @brief assertRangesMutuallyExclusive
     * Simple sanity check to ensure that no bitranges overlap.
     */
    void assertRangesMutuallyExclusive() {
        std::map<unsigned, bool> rangeMap;
        for (const auto& range : ranges) {
            for (size_t i = range.start; i < range.stop; i++) {
                assert(!rangeMap[i] && "Overlapping bitrange detected");
                rangeMap[i] = true;
            }
        }
    }
};

template <typename ISA>
struct Opcode : public Field {
    Opcode(const QString& _name, uint32_t _opcode, unsigned _tokenIndex, BitRange range)
        : name(_name), opcode(_opcode), Field({_tokenIndex}, {range}) {}

    const QString name;
    const uint32_t opcode;
    std::optional<Assembler::Error> apply(const Assembler::SourceLine&, uint32_t& instruction) const override {
        instruction |= ranges[0].apply(opcode);
    }
    std::optional<Assembler::Error> decode(const uint32_t instruction, Assembler::LineTokens& line) const override {
        line.push_back(name);
    }

    bool operator==(uint32_t instruction) const {}
};

template <typename ISA>
struct Reg : public Field {
    Reg(unsigned _tokenIndex, BitRange range) : Field({_tokenIndex}, {range}) {}
    std::optional<Assembler::Error> apply(const Assembler::SourceLine& line, uint32_t& instruction) const override {
        bool success;
        const QString& regToken = line.tokens[tokenIndexes[0]];
        const uint32_t reg = ISA::instance()->getRegisterNumber(regToken, success);
        if (!success) {
            return Assembler::Error(line.source_line, "Unknown register '" + regToken + "'");
        }
        instruction |= ranges[0].apply(reg);
    }
    std::optional<Assembler::Error> decode(const uint32_t instruction, Assembler::LineTokens& line) const override {
        const unsigned regNumber = ranges[0].decode(instruction);
        const QString registerName = ISA::instance()->getRegisterName(regNumber);
        if (registerName.isEmpty()) {
            return Assembler::Error(0, "Unknown register number '" + QString::number(regNumber) + "'");
        }
        line.append(registerName);
    }
};

template <typename ISA>
struct Imm : public Field {
    Imm(const std::vector<unsigned> _tokenIndexes, const std::vector<BitRange>& _ranges)
        : Field(_tokenIndexes, _ranges) {}

    unsigned toUint() {}
    int toInt() {}

    std::optional<Assembler::Error> apply(const Assembler::SourceLine& line, uint32_t& instruction) const override {}
    std::optional<Assembler::Error> decode(const uint32_t instruction, Assembler::LineTokens& line) const override {}
};

template <typename ISA>
class Instruction {
public:
    Instruction(Opcode<ISA> opcode, std::vector<Field> fields)
        : m_opcode(opcode), m_expectedTokens(1 /*opcode*/ + fields.size()) {
        m_assembler = [=](const Assembler::SourceLine& line) {
            uint32_t instruction = 0;
            m_opcode.apply(line, instruction);
            for (const auto& field : fields)
                field.apply(line, instruction);
            return instruction;
        };
        m_disassembler = [=](uint32_t instruction) {
            Assembler::LineTokens line;
            m_opcode.decode(instruction, line);
            for (const auto& field : fields)
                field.decode(instruction, line);
            return line;
        };
    }

    AssembleRes assemble(const Assembler::SourceLine& line) {
        if (line.tokens.length() != m_expectedTokens) {
            return Assembler::Error(line.source_line, "Instruction '" + m_opcode.name + "' expects " +
                                                          QString::number(m_expectedTokens - 1) + " tokens, but got " +
                                                          QString::number(line.tokens.length() - 1));
        }

        return m_assembler(line);
    }
    DisassembleRes disassemble(uint32_t instruction) { return m_disassembler(instruction); }

private:
    std::function<AssembleRes(const Assembler::SourceLine&)> m_assembler;
    std::function<DisassembleRes(uint32_t)> m_disassembler;

    Opcode<ISA> m_opcode;
    const int m_expectedTokens;
};

}  // namespace Ripes
