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

struct BitRangeBase {
  virtual Instr_T apply(Instr_T value) const = 0;
  virtual Instr_T decode(Instr_T instruction) const = 0;
  virtual unsigned start() const = 0;
  virtual unsigned stop() const = 0;
  virtual unsigned width() const = 0;
};

/// start/stop values for bitranges are inclusive..
template <unsigned _start, unsigned _stop, unsigned _N = 32>
struct BitRange : public BitRangeBase {
  static_assert(isPowerOf2(_N) && "Bitrange N must be power of 2");
  static_assert(_start <= _stop && _stop < _N && "invalid range");

  unsigned start() const override { return _start; }
  unsigned stop() const override { return _stop; }
  /*constexpr*/ unsigned width() const override { return _stop - _start + 1; }
  const Instr_T mask = vsrtl::generateBitmask(width());

  Instr_T apply(Instr_T value) const override {
    return (value & mask) << _start;
  }
  Instr_T decode(Instr_T instruction) const override {
    return (instruction >> _start) & mask;
  }

  bool operator==(const BitRange &other) const {
    return this->start == other.start && this->stop == other.stop;
  }
  bool operator<(const BitRange &other) const {
    return (this->start == other.start) ? this->stop < other.stop
                                        : this->start < other.start;
  }
};

template <typename _BitRange>
struct OpPart;

struct OpPartBase {
  template <typename BitRange>
  OpPartBase(unsigned _value, BitRange _range) : range(_range), value(_value) {}
  template <typename BitRange>
  OpPartBase(const OpPart<BitRange> &other)
      : range(other.range), value(other.value) {}
  const std::unique_ptr<BitRangeBase> range;
  const unsigned value;
  bool matches(Instr_T instruction) const {
    return range->decode(instruction) == value;
  }
};

/** @brief OpPart
 * A segment of an operation-identifying field of an instruction.
 */
template <typename _BitRange>
struct OpPart : public OpPartBase {
  using BitRange = _BitRange;

  OpPart(unsigned _value, const BitRange &_range)
      : OpPartBase(_value, _range) {}
  //  bool matches(Instr_T instruction) const {
  //    return getRange().decode(instruction) == value;
  //  }

  static BitRange getRange() { return BitRange(); }

  bool operator==(const OpPart &other) const {
    return this->value == other.value && this->range == other.range;
  }
  bool operator<(const OpPart &other) const {
    if (this->range == other.range)
      return this->value < other.value;
    return this->range < other.range;
  }
};

struct FieldBase;

struct FieldLinkRequest {
  FieldBase const *field = nullptr;
  QString symbol = QString();
  QString relocation = QString();
};

struct FieldBase {
  FieldBase(unsigned _tokenIndex) : tokenIndex(_tokenIndex) {}
  virtual ~FieldBase() = default;
  virtual std::optional<Error>
  apply(const TokenizedSrcLine &line, Instr_T &instruction,
        FieldLinkRequest &linksWithSymbol) const = 0;
  virtual std::optional<Error> decode(const Instr_T instruction,
                                      const Reg_T address,
                                      const ReverseSymbolMap &symbolMap,
                                      LineTokens &line) const = 0;

  const unsigned tokenIndex;
};

template <typename... BitRanges>
struct Field : public FieldBase {
  /// Return the set of bitranges which constitutes this field.
  //  virtual std::vector<BitRange> bitRanges() const = 0;
  virtual std::tuple<BitRanges...> bitRanges() const = 0;
};

/**
 * @brief The InstrRes struct is returned by any assembler. Contains the
 * assembled instruction alongside a flag noting whether the instruction needs
 * additional linkage (ie. for symbol resolution).
 */
struct InstrRes {
  Instr_T instruction = 0;
  FieldLinkRequest linksWithSymbol;
};

using AssembleRes = Result<InstrRes>;

struct OpcodeBase {
  virtual unsigned partCount() const = 0;
  virtual const std::vector<OpPartBase> &getOpParts() const = 0;
};

template <typename... OpParts>
struct Opcode : public OpcodeBase, public Field<typename OpParts::BitRange...> {
  /**
   * @brief Opcode
   * The opcode is assumed to always be the first token within an assembly
   * instruction
   * @param name: Name of operation
   * @param fields: list of OpParts corresponding to the identifying elements of
   * the opcode.
   */
  Opcode(const Token &_name, const std::tuple<OpParts...> &_opParts)
      : Field<typename OpParts::BitRange...>(0), name(_name),
        opParts(_opParts) {}

  std::optional<Error> apply(const TokenizedSrcLine &, Instr_T &instruction,
                             FieldLinkRequest &) const override {
    for (const auto &opPart : opParts) {
      instruction |= opPart.range->apply(opPart.value);
    }
    return std::nullopt;
  }
  std::optional<Error> decode(const Instr_T, const Reg_T /*address*/,
                              const ReverseSymbolMap &,
                              LineTokens &line) const override {
    line.push_back(name);
    return std::nullopt;
  }

  std::tuple<typename OpParts::BitRange...> bitRanges() const override {
    std::tuple<typename OpParts::BitRange...> ranges;
    std::transform(opParts.begin(), opParts.end(), std::back_inserter(ranges),
                   [&](auto opPart) { return opPart.range; });
    return ranges;
  }

  template <typename OpPart>
  inline unsigned one() {
    return 1;
  }
  unsigned partCount() const override { return (one<OpParts>() + ...); }

  const Token name;
  const std::vector<OpPartBase> opParts;
};

struct RegBase {
  virtual const QString &getRegsd() const = 0;
};

template <typename BitRange>
struct Reg : public RegBase, public Field<BitRange> {
  /**
   * @brief Reg
   * @param tokenIndex: Index within a list of decoded instruction tokens that
   * corresponds to the register index
   * @param range: range in instruction field containing register index value
   */
  Reg(const ISAInfoBase *isa, unsigned _tokenIndex, const BitRange &range)
      : Field<BitRange>(_tokenIndex), m_range(range), m_isa(isa) {}
  Reg(const ISAInfoBase *isa, unsigned _tokenIndex, unsigned _start,
      unsigned _stop)
      : Field<BitRange>(_tokenIndex), m_range({_start, _stop}), m_isa(isa) {}
  Reg(const ISAInfoBase *isa, unsigned _tokenIndex, const BitRange &range,
      const QString &_regsd)
      : Field<BitRange>(_tokenIndex), m_range(range), m_isa(isa),
        regsd(_regsd) {}
  Reg(const ISAInfoBase *isa, unsigned _tokenIndex, unsigned _start,
      unsigned _stop, const QString &_regsd)
      : Field<BitRange>(_tokenIndex), m_range({_start, _stop}), m_isa(isa),
        regsd(_regsd) {}
  std::optional<Error> apply(const TokenizedSrcLine &line, Instr_T &instruction,
                             FieldLinkRequest &) const override {
    bool success;
    const QString &regToken = line.tokens[this->tokenIndex];
    const unsigned reg = m_isa->regNumber(regToken, success);
    if (!success) {
      return Error(line, "Unknown register '" + regToken + "'");
    }
    instruction |= m_range.apply(reg);
    return std::nullopt;
  }
  std::optional<Error> decode(const Instr_T instruction,
                              const Reg_T /*address*/, const ReverseSymbolMap &,
                              LineTokens &line) const override {
    const unsigned regNumber = m_range.decode(instruction);
    const Token registerName(m_isa->regName(regNumber));
    if (registerName.isEmpty()) {
      return Error(0, "Unknown register number '" + QString::number(regNumber) +
                          "'");
    }
    line.push_back(registerName);
    return std::nullopt;
  }

  std::vector<BitRange> bitRanges() const override { return {m_range}; }

  const QString &getRegsd() const override { return regsd; }

  const BitRange m_range;
  const ISAInfoBase *m_isa;
  const QString regsd = "reg";
};

template <typename _BitRange>
struct ImmPart {
  using BitRange = _BitRange;

  ImmPart(unsigned _offset, const BitRange &_range)
      : offset(_offset), range(_range) {}
  void apply(const Instr_T value, Instr_T &instruction) const {
    instruction |= range.apply(value >> offset);
  }
  void decode(Instr_T &value, const Instr_T instruction) const {
    value |= range.decode(instruction) << offset;
  }
  const unsigned offset;
  const BitRange range;
};

struct ImmBase {
  virtual unsigned getWidth() const = 0;
};

template <typename... ImmParts>
struct Imm : public ImmBase, public Field<typename ImmParts::BitRange...> {
  using Reg_T_S = typename std::make_signed<Reg_T>::type;
  using Reg_T_U = typename std::make_unsigned<Reg_T>::type;

  enum class Repr { Unsigned, Signed, Hex };
  static Radix reprToRadix(Repr repr) {
    if (repr == Repr::Unsigned)
      return Radix::Unsigned;
    if (repr == Repr::Signed)
      return Radix::Signed;
    if (repr == Repr::Hex)
      return Radix::Hex;
    return Radix::Unsigned;
  }
  enum class SymbolType { None, Relative, Absolute };
  /**
   * @brief Imm
   * @param _tokenIndex: Index within a list of decoded instruction tokens that
   * corresponds to the immediate
   * @param _width: bit-width of the immediate
   * @param _repr: Representation of the immediate
   * @param _parts: (ordered) list of ranges corresponding to fields of the
   * immediate
   * @param _symbolType: Set if this immediate refers to a relative or absolute
   * symbol.
   * @param _symbolTransformer: Optional function used to process the immediate
   * provided by a symbol value, before the immediate value is applied.
   */
  Imm(unsigned _tokenIndex, unsigned _width, Repr _repr,
      const std::tuple<ImmParts...> &_parts,
      SymbolType _symbolType = SymbolType::None,
      const std::function<Reg_T(Reg_T)> &_symbolTransformer = {})
      : Field<typename ImmParts::BitRange...>(_tokenIndex), parts(_parts),
        width(_width), repr(_repr), symbolType(_symbolType),
        symbolTransformer(_symbolTransformer) {}

  int64_t getImm(const QString &immToken, bool &success,
                 ImmConvInfo &convInfo) const {
    return repr == Repr::Signed
               ? getImmediateSext32(immToken, success, &convInfo)
               : getImmediate(immToken, success, &convInfo);
  }

  std::optional<Error> apply(const TokenizedSrcLine &line, Instr_T &instruction,
                             FieldLinkRequest &linksWithSymbol) const override {
    bool success;
    const Token &immToken = line.tokens[this->tokenIndex];
    ImmConvInfo convInfo;
    Reg_T_S value = getImm(immToken, success, convInfo);

    if (!success) {
      // Could not directly resolve immediate. Register it as a symbol to link
      // to.
      linksWithSymbol.field = this;
      linksWithSymbol.symbol = immToken;
      linksWithSymbol.relocation = immToken.relocation();
      return {};
    }

    if (auto res = checkFitsInWidth(value, line, convInfo, immToken);
        res.isError())
      return res.error();

    for (const auto &part : parts) {
      part.apply(value, instruction);
    }
    return std::nullopt;
  }

  Result<> checkFitsInWidth(Reg_T_S value, const Location &sourceLine,
                            ImmConvInfo &convInfo,
                            QString token = QString()) const {
    bool err = false;
    if (repr != Repr::Signed) {
      if (!isUInt(width, value)) {
        err = true;
        if (token.isEmpty())
          token = QString::number(static_cast<Reg_T_U>(value));
      }
    } else {

      // In case of a bitwize (binary or hex) radix, interpret the value as
      // legal if it fits in the width of this immediate (equal to an unsigned
      // immediate check). e.g. a signed immediate value of 12 bits must be able
      // to accept 0xAFF.
      bool isBitwize =
          convInfo.radix == Radix::Hex || convInfo.radix == Radix::Binary;
      if (isBitwize) {
        err = !isUInt(width, value);
      }

      if (!isBitwize || (isBitwize && err)) {
        // A signed representation using an integer value in assembly OR a
        // negative bitwize value which is represented in its full length (e.g.
        // 0xFFFFFFF1).
        err = !isInt(width, value);
      }

      if (err)
        if (token.isEmpty())
          token = QString::number(static_cast<Reg_T_S>(value));
    }

    if (err) {
      return Error(sourceLine, "Immediate value '" + token +
                                   "' does not fit in " +
                                   QString::number(width) + " bits");
    }

    return Result<>::def();
  }

  Result<> applySymbolResolution(const Location &loc, Reg_T symbolValue,
                                 Instr_T &instruction, Reg_T address) const {
    ImmConvInfo convInfo;
    convInfo.radix = reprToRadix(repr);
    Reg_T adjustedValue = symbolValue;
    if (symbolType == SymbolType::Relative)
      adjustedValue -= address;

    if (symbolTransformer)
      adjustedValue = symbolTransformer(adjustedValue);

    if (auto res = checkFitsInWidth(adjustedValue, loc, convInfo);
        res.isError())
      return res.error();

    for (const auto &part : parts)
      part.apply(adjustedValue, instruction);

    return Result<>::def();
  }

  std::optional<Error> decode(const Instr_T instruction, const Reg_T address,
                              const ReverseSymbolMap &symbolMap,
                              LineTokens &line) const override {
    Instr_T reconstructed = 0;
    for (const auto &part : parts) {
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
      const Reg_T symbolAddress =
          value + (symbolType == SymbolType::Absolute ? 0 : address);
      if (symbolMap.count(symbolAddress)) {
        line.push_back("<" + symbolMap.at(symbolAddress).v + ">");
      }
    }

    return std::nullopt;
  }

  std::tuple<typename ImmParts::BitRange...> bitRanges() const override {
    std::tuple<typename ImmParts::BitRange...> ranges;
    std::transform(parts.begin(), parts.end(), std::back_inserter(ranges),
                   [&](auto opPart) { return opPart.range; });
    return ranges;
  }

  unsigned getWidth() const override { return width; }

  const std::tuple<ImmParts...> parts;
  const unsigned width;
  const Repr repr;
  const SymbolType symbolType;
  const std::function<Reg_T(Reg_T)> symbolTransformer;
};

struct InstructionBase {
  virtual const OpcodeBase &getOpcode() const = 0;
  virtual bool hasExtraMatchConds() const = 0;
  virtual bool matchesWithExtras(Instr_T instr) const = 0;
  virtual const QString &name() const = 0;
};

template <typename Opcode, typename... Fields>
class Instruction : public InstructionBase {
public:
  Instruction(const Opcode &opcode,
              const std::vector<std::shared_ptr<FieldBase>> &fields)
      : m_opcode(opcode), m_expectedTokens(1 /*opcode*/ + fields.size()),
        m_fields(fields) {
    m_assembler = [](const Instruction *_this, const TokenizedSrcLine &line) {
      InstrRes res;
      _this->m_opcode.apply(line, res.instruction, res.linksWithSymbol);
      for (const auto &field : _this->m_fields) {
        auto err = field->apply(line, res.instruction, res.linksWithSymbol);
        if (err) {
          return AssembleRes(err.value());
        }
      }
      return AssembleRes(res);
    };
    m_disassembler = [](const Instruction *_this, const Instr_T instruction,
                        const Reg_T address,
                        const ReverseSymbolMap &symbolMap) {
      LineTokens line;
      _this->m_opcode.decode(instruction, address, symbolMap, line);
      for (const auto &field : _this->m_fields) {
        if (auto error = field->decode(instruction, address, symbolMap, line)) {
          return Result<LineTokens>(*error);
        }
      }
      return Result<LineTokens>(line);
    };

    verify();

    // Sort fields by token index. This lets the disassembler naively write each
    // parsed field in the order of m_fields, ensuring that the disassembled
    // tokens appear in correct order, without having to manually check this on
    // each disassemble call.
    std::sort(m_fields.begin(), m_fields.end(),
              [](const auto &field1, const auto &field2) {
                return field1->tokenIndex < field2->tokenIndex;
              });
  }

  AssembleRes assemble(const TokenizedSrcLine &line) const {
    QString Hint = "";

    for (const auto &field : m_fields) {
      if (auto *immField = dynamic_cast<ImmBase *>(field.get()))
        Hint = Hint + " [Imm(" + QString::number(immField->getWidth()) + ")]";
      else if (auto *regField = dynamic_cast<RegBase *>(field.get())) {
        Hint = Hint + " [" + regField->getRegsd() + "]";
      }
    }
    if (line.tokens.size() != m_expectedTokens) {
      return Error(line, "Instruction " + m_opcode.name + Hint + " expects " +
                             QString::number(m_expectedTokens - 1) +
                             " arguments, but got " +
                             QString::number(line.tokens.size() - 1));
    }
    return m_assembler(this, line);
  }

  Result<LineTokens> disassemble(const Instr_T instruction, const Reg_T address,
                                 const ReverseSymbolMap &symbolMap) const {
    return m_disassembler(this, instruction, address, symbolMap);
  }

  const OpcodeBase &getOpcode() const { return m_opcode; }
  const QString &name() const { return m_opcode.name; }
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
    for (auto &field : m_fields) {
      if (tokenIndexes.count(field->tokenIndex))
        assert(false && "Duplicate token indices!");
      tokenIndexes.insert(field->tokenIndex);
    }
    for (size_t i = 1; i < m_fields.size() + 1; ++i) {
      assert(tokenIndexes.count(i) && "Mismatched token indexes, should have "
                                      "registerred 1:N sequential tokens");
    }

    std::tuple<typename Fields::BitRange...> bitRanges;
    for (auto &field : m_fields) {
      auto fieldBitRanges = field->bitRanges();
      std::copy(fieldBitRanges.begin(), fieldBitRanges.end(),
                std::back_inserter(bitRanges));
    }
    auto opcodeBitRanges = m_opcode.bitRanges();
    std::copy(opcodeBitRanges.begin(), opcodeBitRanges.end(),
              std::back_inserter(bitRanges));

    // 1.
    std::set<unsigned> registeredBits;
    for (auto &range : bitRanges) {
      for (unsigned i = range.start; i <= range.stop; ++i) {
        assert(registeredBits.count(i) == 0 &&
               "Bit already registerred by some other field");
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

  void addExtraMatchCond(const std::function<bool(Instr_T)> &f) {
    m_extraMatchConditions.push_back(f);
  }
  bool hasExtraMatchConds() const { return !m_extraMatchConditions.empty(); }
  bool matchesWithExtras(Instr_T instr) const {
    return llvm::all_of(m_extraMatchConditions,
                        [&](const auto &f) { return f(instr); });
  }

private:
  std::function<AssembleRes(const Instruction *, const TokenizedSrcLine &)>
      m_assembler;
  std::function<Result<LineTokens>(const Instruction *, const Instr_T,
                                   const Reg_T, const ReverseSymbolMap &)>
      m_disassembler;

  const Opcode m_opcode;
  const int m_expectedTokens;
  std::tuple<Fields...> m_fields;
  unsigned m_byteSize = -1;

  /// An optional set of disassembler match conditions, if the default
  /// opcode-based matching is insufficient.
  std::vector<std::function<bool(Instr_T)>> m_extraMatchConditions;
};

using InstrMap = std::map<QString, std::shared_ptr<InstructionBase>>;

using InstrVec = std::vector<std::shared_ptr<InstructionBase>>;

} // namespace Assembler

} // namespace Ripes
