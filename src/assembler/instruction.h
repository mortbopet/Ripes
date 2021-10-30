#pragma once

#include "../binutils.h"
#include "assembler_defines.h"
#include "parserutilities.h"

#include <QString>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>

#include "STLExtras.h"
#include "binutils.h"
#include "isa/isainfo.h"
#include "math.h"

namespace Ripes {
namespace Assembler {

/// start/stop values for bitranges are inclusive..
struct BitRange {
    constexpr BitRange(unsigned _start, unsigned _stop, unsigned _N = 32) : start(_start), stop(_stop), N(_N) {
        assert(isPowerOf2(_N) && "Bitrange N must be power of 2");
        assert(_start <= _stop && _stop < _N && "invalid range");
    }
    constexpr unsigned width() const { return stop - start + 1; }
    const unsigned start, stop, N;
    const Instr_T mask = vsrtl::generateBitmask(width());

    Instr_T apply(Instr_T value) const { return (value & mask) << start; }
    Instr_T decode(Instr_T instruction) const { return (instruction >> start) & mask; }

    bool operator==(const BitRange& other) const { return this->start == other.start && this->stop == other.stop; }
    bool operator<(const BitRange& other) const {
        return (this->start == other.start) ? this->stop < other.stop : this->start < other.start;
    }
};

/** @brief OpPart
 * A segment of an operation-identifying field of an instruction.
 */
struct OpPart {
    OpPart(unsigned _value, BitRange _range) : value(_value), range(_range) {}
    OpPart(unsigned _value, unsigned _start, unsigned _stop) : value(_value), range({_start, _stop}) {}
    bool matches(Instr_T instruction) const { return range.decode(instruction) == value; }
    const unsigned value;
    const BitRange range;

    bool operator==(const OpPart& other) const { return this->value == other.value && this->range == other.range; }
    bool operator<(const OpPart& other) const {
        if (this->range == other.range)
            return this->value < other.value;
        return this->range < other.range;
    }
};

template <typename Reg_T>
struct Field;

template <typename Reg_T>
struct FieldLinkRequest {
    Field<Reg_T> const* field = nullptr;
    QString symbol = QString();
    QString relocation = QString();
};

template <typename Reg_T>
struct Field {
    Field(unsigned _tokenIndex) : tokenIndex(_tokenIndex) {}
    virtual ~Field() = default;
    virtual std::optional<Error> apply(const TokenizedSrcLine& line, Instr_T& instruction,
                                       FieldLinkRequest<Reg_T>& linksWithSymbol) const = 0;
    virtual std::optional<Error> decode(const Instr_T instruction, const Reg_T address,
                                        const ReverseSymbolMap& symbolMap, LineTokens& line) const = 0;

    /// Return the set of bitranges which consitutes this field.
    virtual std::vector<BitRange> bitRanges() const = 0;
    const unsigned tokenIndex;
};

/**
 * @brief The InstrRes struct is returned by any assembler. Contains the assembled instruction alongside a flag noting
 * whether the instruction needs additional linkage (ie. for symbol resolution).
 */
template <typename Reg_T>
struct InstrRes {
    Instr_T instruction = 0;
    FieldLinkRequest<Reg_T> linksWithSymbol;
};

template <typename Reg_T>
using AssembleRes = std::variant<Error, InstrRes<Reg_T>>;

using PseudoExpandRes = std::variant<Error, std::optional<std::vector<LineTokens>>>;
using DisassembleRes = std::variant<Error, LineTokens>;

template <typename Reg_T>
struct Opcode : public Field<Reg_T> {
    /**
     * @brief Opcode
     * The opcode is assumed to always be the first token within an assembly instruction
     * @param name: Name of operation
     * @param fields: list of OpParts corresponding to the identifying elements of the opcode.
     */
    Opcode(const Token& _name, std::vector<OpPart> _opParts) : Field<Reg_T>(0), name(_name), opParts(_opParts) {}

    std::optional<Error> apply(const TokenizedSrcLine&, Instr_T& instruction, FieldLinkRequest<Reg_T>&) const override {
        for (const auto& opPart : opParts) {
            instruction |= opPart.range.apply(opPart.value);
        }
        return std::nullopt;
    }
    std::optional<Error> decode(const Instr_T, const Reg_T /*address*/, const ReverseSymbolMap&,
                                LineTokens& line) const override {
        line.push_back(name);
        return std::nullopt;
    }

    std::vector<BitRange> bitRanges() const override {
        std::vector<BitRange> ranges;
        std::transform(opParts.begin(), opParts.end(), std::back_inserter(ranges),
                       [&](auto opPart) { return opPart.range; });
        return ranges;
    }

    const Token name;
    const std::vector<OpPart> opParts;
};

template <typename Reg_T>
struct Reg : public Field<Reg_T> {
    /**
     * @brief Reg
     * @param tokenIndex: Index within a list of decoded instruction tokens that corresponds to the register index
     * @param range: range in instruction field containing register index value
     */
    Reg(const ISAInfoBase* isa, unsigned _tokenIndex, BitRange range)
        : Field<Reg_T>(_tokenIndex), m_range(range), m_isa(isa) {}
    Reg(const ISAInfoBase* isa, unsigned _tokenIndex, unsigned _start, unsigned _stop)
        : Field<Reg_T>(_tokenIndex), m_range({_start, _stop}), m_isa(isa) {}
    Reg(const ISAInfoBase* isa, unsigned _tokenIndex, BitRange range, QString _regsd)
        : Field<Reg_T>(_tokenIndex), m_range(range), m_isa(isa), regsd(_regsd) {}
    Reg(const ISAInfoBase* isa, unsigned _tokenIndex, unsigned _start, unsigned _stop, QString _regsd)
        : Field<Reg_T>(_tokenIndex), m_range({_start, _stop}), m_isa(isa), regsd(_regsd) {}
    std::optional<Error> apply(const TokenizedSrcLine& line, Instr_T& instruction,
                               FieldLinkRequest<Reg_T>&) const override {
        bool success;
        const QString& regToken = line.tokens[this->tokenIndex];
        const unsigned reg = m_isa->regNumber(regToken, success);
        if (!success) {
            return Error(line.sourceLine, "Unknown register '" + regToken + "'");
        }
        instruction |= m_range.apply(reg);
        return std::nullopt;
    }
    std::optional<Error> decode(const Instr_T instruction, const Reg_T /*address*/, const ReverseSymbolMap&,
                                LineTokens& line) const override {
        const unsigned regNumber = m_range.decode(instruction);
        const Token registerName = m_isa->regName(regNumber);
        if (registerName.isEmpty()) {
            return Error(0, "Unknown register number '" + QString::number(regNumber) + "'");
        }
        line.push_back(registerName);
        return std::nullopt;
    }

    std::vector<BitRange> bitRanges() const override { return {m_range}; }

    const BitRange m_range;
    const ISAInfoBase* m_isa;
    const QString regsd = "reg";
};

struct ImmPart {
    ImmPart(unsigned _offset, BitRange _range) : offset(_offset), range(_range) {}
    ImmPart(unsigned _offset, unsigned _start, unsigned _stop) : offset(_offset), range({_start, _stop}) {}
    void apply(const Instr_T value, Instr_T& instruction) const { instruction |= range.apply(value >> offset); }
    void decode(Instr_T& value, const Instr_T instruction) const { value |= range.decode(instruction) << offset; }
    const unsigned offset;
    const BitRange range;
};

template <typename Reg_T>
struct Imm : public Field<Reg_T> {
    using Reg_T_S = typename std::make_signed<Reg_T>::type;
    using Reg_T_U = typename std::make_unsigned<Reg_T>::type;

    enum class Repr { Unsigned, Signed, Hex };
    enum class SymbolType { None, Relative, Absolute };
    /**
     * @brief Imm
     * @param _tokenIndex: Index within a list of decoded instruction tokens that corresponds to the immediate
     * @param _width: bit-width of the immediate
     * @param _repr: Representation of the immediate
     * @param _parts: (ordered) list of ranges corresponding to fields of the immediate
     * @param _symbolType: Set if this immediate refers to a relative or absolute symbol.
     * @param _symbolTransformer: Optional function used to process the immediate provided by a symbol value, before the
     * immediate value is applied.
     */
    Imm(unsigned _tokenIndex, unsigned _width, Repr _repr, const std::vector<ImmPart>& _parts,
        SymbolType _symbolType = SymbolType::None, const std::function<Reg_T(Reg_T)>& _symbolTransformer = {})
        : Field<Reg_T>(_tokenIndex),
          parts(_parts),
          width(_width),
          repr(_repr),
          symbolType(_symbolType),
          symbolTransformer(_symbolTransformer) {}

    int64_t getImm(const QString& immToken, bool& success) const {
        return repr == Repr::Signed ? getImmediateSext32(immToken, success) : getImmediate(immToken, success);
    }

    std::optional<Error> apply(const TokenizedSrcLine& line, Instr_T& instruction,
                               FieldLinkRequest<Reg_T>& linksWithSymbol) const override {
        bool success;
        const Token& immToken = line.tokens[this->tokenIndex];
        Reg_T_S value = getImm(immToken, success);

        if (!success) {
            // Could not directly resolve immediate. Register it as a symbol to link to.
            linksWithSymbol.field = this;
            linksWithSymbol.symbol = immToken;
            linksWithSymbol.relocation = immToken.relocation();
            return {};
        }

        if (auto err = checkFitsInWidth(value, line.sourceLine)) {
            return err;
        }

        for (const auto& part : parts) {
            part.apply(value, instruction);
        }
        return std::nullopt;
    }

    std::optional<Error> checkFitsInWidth(Reg_T_S value, unsigned sourceLine) const {
        if (!(repr == Repr::Signed ? isInt(width, value) : (isUInt(width, value)))) {
            const QString v = repr == Repr::Signed ? QString::number(static_cast<Reg_T_S>(value))
                                                   : QString::number(static_cast<Reg_T_U>(value));
            return Error(sourceLine, "Immediate value '" + v + "' does not fit in " + QString::number(width) + " bits");
        }
        return std::nullopt;
    }

    std::optional<Error> applySymbolResolution(Reg_T symbolValue, Instr_T& instruction, const Reg_T address,
                                               unsigned sourceLine) const {
        Reg_T adjustedValue = symbolValue;
        if (symbolType == SymbolType::Relative) {
            adjustedValue -= address;
        }

        if (symbolTransformer) {
            adjustedValue = symbolTransformer(adjustedValue);
        }

        if (auto err = checkFitsInWidth(adjustedValue, sourceLine)) {
            return err;
        }

        for (const auto& part : parts) {
            part.apply(adjustedValue, instruction);
        }
        return std::nullopt;
    }

    std::optional<Error> decode(const Instr_T instruction, const Reg_T address, const ReverseSymbolMap& symbolMap,
                                LineTokens& line) const override {
        Instr_T reconstructed = 0;
        for (const auto& part : parts) {
            part.decode(reconstructed, instruction);
        }
        if (repr == Repr::Signed) {
            line.push_back(QString::number(vsrtl::signextend(reconstructed, width)));
        } else if (repr == Repr::Unsigned) {
            line.push_back(QString::number(reconstructed));
        } else {
            line.push_back("0x" + QString::number(reconstructed, 16));
        }

        if (symbolType != SymbolType::None) {
            const int value = vsrtl::signextend(reconstructed, width);
            const Reg_T symbolAddress = value + (symbolType == SymbolType::Absolute ? 0 : address);
            if (symbolMap.count(symbolAddress)) {
                line.push_back("<" + symbolMap.at(symbolAddress).v + ">");
            }
        }

        return std::nullopt;
    }

    std::vector<BitRange> bitRanges() const override {
        std::vector<BitRange> ranges;
        std::transform(parts.begin(), parts.end(), std::back_inserter(ranges),
                       [&](auto opPart) { return opPart.range; });
        return ranges;
    }

    const std::vector<ImmPart> parts;
    const unsigned width;
    const Repr repr;
    const SymbolType symbolType;
    const std::function<Reg_T(Reg_T)> symbolTransformer;
};

template <typename Reg_T>
class Instruction {
public:
    Instruction(Opcode<Reg_T> opcode, const std::vector<std::shared_ptr<Field<Reg_T>>>& fields)
        : m_opcode(opcode), m_expectedTokens(1 /*opcode*/ + fields.size()), m_fields(fields) {
        m_assembler = [](const Instruction* _this, const TokenizedSrcLine& line) {
            InstrRes<Reg_T> res;
            _this->m_opcode.apply(line, res.instruction, res.linksWithSymbol);
            for (const auto& field : _this->m_fields) {
                auto err = field->apply(line, res.instruction, res.linksWithSymbol);
                if (err) {
                    return AssembleRes<Reg_T>(err.value());
                }
            }
            return AssembleRes<Reg_T>(res);
        };
        m_disassembler = [](const Instruction* _this, const Instr_T instruction, const Reg_T address,
                            const ReverseSymbolMap& symbolMap) {
            LineTokens line;
            _this->m_opcode.decode(instruction, address, symbolMap, line);
            for (const auto& field : _this->m_fields) {
                if (auto error = field->decode(instruction, address, symbolMap, line)) {
                    return DisassembleRes(*error);
                }
            }
            return DisassembleRes(line);
        };

        verify();

        // Sort fields by token index. This lets the disassembler naively write each parsed field in the order of
        // m_fields, ensuring that the disassembled tokens appear in correct order, without having to manually check
        // this on each disassemble call.
        std::sort(m_fields.begin(), m_fields.end(),
                  [](const auto& field1, const auto& field2) { return field1->tokenIndex < field2->tokenIndex; });
    }

    AssembleRes<Reg_T> assemble(const TokenizedSrcLine& line) const {
        QString Hint = "";

        for (const auto& field : m_fields) {
            if (auto* immField = dynamic_cast<Imm<Reg_T>*>(field.get()))
                Hint = Hint + " [Imm(" + QString::number(immField->width) + ")]";
            else if (auto* regField = dynamic_cast<Reg<Reg_T>*>(field.get())) {
                Hint = Hint + " [" + regField->regsd + "]";
            }
        }
        if (line.tokens.size() != m_expectedTokens) {
            return Error(line.sourceLine, "Instruction " + m_opcode.name + Hint + " expects " +
                                              QString::number(m_expectedTokens - 1) + " arguments, but got " +
                                              QString::number(line.tokens.size() - 1));
        }
        return m_assembler(this, line);
    }

    DisassembleRes disassemble(const Instr_T instruction, const Reg_T address,
                               const ReverseSymbolMap& symbolMap) const {
        return m_disassembler(this, instruction, address, symbolMap);
    }

    const Opcode<Reg_T>& getOpcode() const { return m_opcode; }
    const QString& name() const { return m_opcode.name; }
    /**
     * @brief size
     * @return size of assembled instruction, in bytes.
     */
    unsigned size() const { return m_byteSize; }

    /// Verify that the bitranges specified for this operation:
    /// 1. do not overlap
    /// 2. fully defines the instruction (no bits are unaccounted for)
    /// 3. is byte aligned
    /// Using this information, we also set the size of this instruction.
    void verify() {
        // sanity check the provided token indexes
        std::set<int> tokenIndexes;
        for (auto& field : m_fields) {
            if (tokenIndexes.count(field->tokenIndex))
                assert(false && "Duplicate token indices!");
            tokenIndexes.insert(field->tokenIndex);
        }
        for (size_t i = 1; i < m_fields.size() + 1; ++i) {
            assert(tokenIndexes.count(i) && "Mismatched token indexes, should have registerred 1:N sequential tokens");
        }

        std::vector<BitRange> bitRanges;
        for (auto& field : m_fields) {
            auto fieldBitRanges = field->bitRanges();
            std::copy(fieldBitRanges.begin(), fieldBitRanges.end(), std::back_inserter(bitRanges));
        }
        auto opcodeBitRanges = m_opcode.bitRanges();
        std::copy(opcodeBitRanges.begin(), opcodeBitRanges.end(), std::back_inserter(bitRanges));

        // 1.
        std::set<unsigned> registeredBits;
        for (auto& range : bitRanges) {
            for (unsigned i = range.start; i <= range.stop; ++i) {
                assert(registeredBits.count(i) == 0 && "Bit already registerred by some other field");
                registeredBits.insert(i);
            }
        }

        // 2.
        assert(registeredBits.count(0) == 1 && "Expected bit 0 to be in bit-range");
        // rbegin due to set being sorted.
        unsigned nBits = registeredBits.size();
        if ((nBits - 1) != *registeredBits.rbegin()) {
            std::string err = "Bits '";
            for (unsigned i = 0; i < nBits; ++i) {
                if (registeredBits.count(i) == 0) {
                    err += std::to_string(i) + ", ";
                }
            }
            std::cerr << err << "\n";
            assert(false);
        }

        // 3.
        assert(nBits % CHAR_BIT == 0 && "Expected instruction to be byte-aligned");
        m_byteSize = nBits / CHAR_BIT;
    }

    void addExtraMatchCond(const std::function<bool(Instr_T)>& f) { m_extraMatchConditions.push_back(f); }
    bool hasExtraMatchConds() const { return !m_extraMatchConditions.empty(); }
    bool matchesWithExtras(Instr_T instr) const {
        return llvm::all_of(m_extraMatchConditions, [&](const auto& f) { return f(instr); });
    }

private:
    std::function<AssembleRes<Reg_T>(const Instruction<Reg_T>*, const TokenizedSrcLine&)> m_assembler;
    std::function<DisassembleRes(const Instruction<Reg_T>*, const Instr_T, const Reg_T, const ReverseSymbolMap&)>
        m_disassembler;

    const Opcode<Reg_T> m_opcode;
    const int m_expectedTokens;
    std::vector<std::shared_ptr<Field<Reg_T>>> m_fields;
    unsigned m_byteSize = -1;

    /// An optional set of disassembler match conditions, if the default opcode-based matching is insufficient.
    std::vector<std::function<bool(Instr_T)>> m_extraMatchConditions;
};

template <typename Reg_T>
using InstrMap = std::map<QString, std::shared_ptr<Instruction<Reg_T>>>;

template <typename Reg_T>
using InstrVec = std::vector<std::shared_ptr<Instruction<Reg_T>>>;

}  // namespace Assembler

}  // namespace Ripes
