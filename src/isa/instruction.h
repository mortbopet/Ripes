#pragma once

#include <climits>
#include <memory>
#include <string_view>
#include <vector>

#include <QList>

#include "../binutils.h"
#include "isa_defines.h"
#include <STLExtras.h>

namespace Ripes {

template <typename T>
constexpr T generateBitmask(T n) {
  using T_S = typename std::make_signed<T>::type;
  using T_U = typename std::make_unsigned<T>::type;
  if (n >= (sizeof(T) * CHAR_BIT)) {
    return static_cast<T_U>(static_cast<T_S>(-1));
  }
  if (n == 0) {
    return 0;
  }
  return T_U((T_U(1) << n) - 1);
}

struct BitRangeStruct {
  unsigned start, stop, N;
  unsigned width() const { return stop - start + 1; }
  Instr_T getMask() const { return generateBitmask(width()); }
  Instr_T decode(Instr_T instruction) const {
    return (instruction >> start) & getMask();
  }
};

/// start/stop values for bitranges are inclusive..
template <unsigned _start, unsigned _stop, unsigned _N = 32>
struct BitRange {
  constexpr static unsigned N() { return _N; }
  constexpr static unsigned start() { return _start; }
  constexpr static unsigned stop() { return _stop; }
  constexpr static unsigned width() { return _stop - _start + 1; }
  constexpr static Instr_T getMask() { return generateBitmask(width()); }

  constexpr static BitRangeStruct getStruct() {
    return BitRangeStruct{_start, _stop, _N};
  }

  constexpr static Instr_T apply(Instr_T value) {
    return (value & getMask()) << _start;
  }
  constexpr static Instr_T decode(Instr_T instruction) {
    return (instruction >> _start) & getMask();
  }

  template <typename OtherBitRange>
  constexpr static bool isEqualTo() {
    return _start == OtherBitRange::start() && _stop == OtherBitRange::stop();
  }
  template <typename OtherBitRange>
  constexpr static bool isLessThan() {
    return (_start == OtherBitRange::start()) ? _stop < OtherBitRange::stop()
                                              : _start < OtherBitRange::start();
  }
};

template <typename... BitRanges>
struct BitRangesImpl : public BitRanges... {
  constexpr static void apply(Instr_T &instruction) {
    (BitRanges::apply(instruction), ...);
  }
  constexpr static void
  retrieveBitRanges(std::vector<BitRangeStruct> &bitRanges) {
    (bitRanges.push_back(BitRanges::getStruct()), ...);
  }
};

struct OpPartStruct {
  unsigned value;
  BitRangeStruct range;
  bool operator<(const OpPartStruct &other) const {
    return range.start < other.range.start;
  }
  bool matches(Instr_T instruction) const {
    return range.decode(instruction) == value;
  }
};

/** @brief OpPart
 * A segment of an operation-identifying field of an instruction.
 */
template <unsigned _value, typename _BitRange>
struct OpPart {
  using BitRange = _BitRange;
  constexpr static unsigned value() { return _value; }
  constexpr static OpPartStruct getStruct() {
    return OpPartStruct{_value, BitRange::getStruct()};
  }

  constexpr static void apply(Instr_T &instruction) {
    instruction |= BitRange::apply(value());
  }
  constexpr static bool matches(Instr_T instruction) {
    return BitRange::decode(instruction) == _value;
  }

  template <typename OtherOpPart>
  constexpr static bool isEqualTo() {
    return _value == OtherOpPart::value() &&
           BitRange::template isEqualTo<OtherOpPart>();
  }
  template <typename OtherOpPart>
  constexpr static bool isLessThan() {
    if (BitRange::template isEqualTo<OtherOpPart>())
      return _value < OtherOpPart::value();
    return BitRange::template isLessThan<OtherOpPart>();
  }
};

template <typename... OpParts>
class OpcodeImpl : public OpParts... {
public:
  using BitRanges = BitRangesImpl<typename OpParts::BitRange...>;

  constexpr static void apply(Instr_T &instruction) {
    (OpParts::apply(instruction), ...);
  }
  constexpr static unsigned numParts() { return sizeof...(OpParts); }
  constexpr static OpPartStruct getOpPart(unsigned partIndex) {
    assert(partIndex < OP_PARTS.size());
    return OP_PARTS[partIndex];
  }
  constexpr static void
  retrieveBitRanges(std::vector<BitRangeStruct> &bitRanges) {
    BitRanges::retrieveBitRanges(bitRanges);
  }

private:
  constexpr static std::array<OpPartStruct, numParts()> OP_PARTS = {
      (OpParts::getStruct(), ...)};
};

struct FieldBase {};

struct FieldLinkRequest {
  FieldBase const *field = nullptr;
  QString symbol = QString();
  QString relocation = QString();
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

template <unsigned _tokenIndex, typename _BitRanges>
struct Field : public FieldBase {
  using BitRanges = _BitRanges;
  constexpr static unsigned tokenIndex() { return _tokenIndex; }
};

template <typename... Fields>
class FieldsImpl : public Fields... {
public:
  /// Verify that the token indices specified for this operation:
  /// 1. do not overlap
  static_assert((Fields::tokenIndex() != ...), "Duplicate token indices!");

  using BitRanges = BitRangesImpl<typename Fields::BitRanges...>;

  constexpr static void apply(const TokenizedSrcLine &tokens,
                              Instr_T &instruction) {
    (Fields::apply(tokens, instruction), ...);
  }
  constexpr static bool decode(const Instr_T instruction, const Reg_T address,
                               const ReverseSymbolMap &symbolMap,
                               LineTokens &line) {
    bool failure = false;
    ((failure |= !Fields::decode(instruction, address, symbolMap, line)), ...);
    return !failure;
  }
  constexpr static unsigned numFields() { return sizeof...(Fields); }
  constexpr static void
  retrieveBitRanges(std::vector<BitRangeStruct> &bitRanges) {
    BitRanges::retrieveBitRanges(bitRanges);
  }

private:
  /// Verify that the token indices specified for this operation:
  /// 2. are sequentially ordered, starting from 0  // 1. sanity check the
  /// provided token indexes
  template <unsigned fieldIndex, typename InnerField, typename... InnerFields>
  struct FieldsVerify : FieldsVerify<fieldIndex + 1, InnerFields...> {
    static_assert(InnerField::tokenIndex() == fieldIndex);
  };
  template <unsigned fieldIndex, typename InnerField>
  struct FieldsVerify<fieldIndex, InnerField> {
    static_assert(InnerField::tokenIndex() == fieldIndex);
  };
  using Verify = FieldsVerify<0, Fields...>;
};

/**
 * @brief Reg
 * @param tokenIndex: Index within a list of decoded instruction tokens that
 * corresponds to the register index
 * @param BitRange: range in instruction field containing register index value
 */
template <unsigned tokenIndex, typename BitRange, typename ISARegInterface>
struct Reg : public Field<tokenIndex, BitRange> {
  Reg(const QString &_regsd) : regsd(_regsd) {}

  constexpr static bool
  apply(const TokenizedSrcLine &line,
        Instr_T &instruction /*, FieldLinkRequest<Reg_T> &*/) {
    const auto &regToken = line.tokens.at(tokenIndex);
    bool success = false;
    unsigned regIndex = ISARegInterface::regNumber(regToken, success);
    if (!success) {
      // TODO: Set error in FieldLinkRequest
      //      return Error(line, "Unknown register '" + regToken + "'");
      return false;
    }
    instruction |= BitRange::apply(regIndex);
    return true;
  }
  static bool decode(const Instr_T instruction, const Reg_T,
                     const ReverseSymbolMap &, LineTokens &line) {
    const unsigned regNumber = BitRange::decode(instruction);
    const Token registerName(ISARegInterface::regName(regNumber));
    if (registerName.isEmpty()) {
      //      return Error(0, "Unknown register number '" +
      //      QString::number(regNumber) +
      //                          "'");
      return false;
    }
    line.push_back(registerName);
    return true;
  }

  const QString regsd = "reg";
};

template <unsigned _offset, typename _BitRange>
struct ImmPart {
  using BitRange = _BitRange;

  constexpr static void apply(const Instr_T value, Instr_T &instruction) {
    instruction |= BitRange::apply(value >> _offset);
  }
  constexpr static void decode(Instr_T &value, const Instr_T instruction) {
    value |= BitRange::decode(instruction) << _offset;
  }
};

template <typename... ImmParts>
struct ImmPartsImpl : public ImmParts... {
  using BitRanges = BitRangesImpl<typename ImmParts::BitRange...>;

  constexpr static void apply(const Instr_T value, Instr_T &instruction) {
    (ImmParts::apply(value, instruction), ...);
  }
  constexpr static void decodeParts(Instr_T &value, const Instr_T instruction) {
    (ImmParts::decode(value, instruction), ...);
  }
};

enum class Repr { Unsigned, Signed, Hex };
enum class SymbolType { None, Relative, Absolute };

// using SymbolTransformer = const std::function<Reg_T(Reg_T)> &;
typedef Reg_T (*SymbolTransformer)(Reg_T);

inline Reg_T defaultTransformer(Reg_T reg) { return reg; }

/**
 * @brief Imm
 * @param tokenIndex: Index within a list of decoded instruction tokens that
 * corresponds to the immediate
 * @param width: bit-width of the immediate
 * @param repr: Representation of the immediate
 * @param symbolType: Set if this immediate refers to a relative or absolute
 * symbol.
 * @param transformer: Optional function used to process the immediate
 * provided by a symbol value, before the immediate value is applied.
 * @param ImmParts: (ordered) list of ranges corresponding to fields of the
 * immediate
 */
template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts,
          SymbolType symbolType, SymbolTransformer transformer>
struct ImmBase : public Field<tokenIndex, typename ImmParts::BitRanges> {
  using Reg_T_S = typename std::make_signed<Reg_T>::type;
  using Reg_T_U = typename std::make_unsigned<Reg_T>::type;

  constexpr static int64_t getImm(const QString &immToken, bool &success,
                                  ImmConvInfo &convInfo) {
    return repr == Repr::Signed
               ? getImmediateSext32(immToken, success, &convInfo)
               : getImmediate(immToken, success, &convInfo);
  }

  static bool checkFitsInWidth(Reg_T_S value, ImmConvInfo &convInfo,
                               QString token = QString()) {
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
      //      return Error(sourceLine, "Immediate value '" + token +
      //                                   "' does not fit in " +
      //                                   QString::number(width) + " bits");
      // TODO: Return error to caller
      return false;
    }

    return true;
  }

  constexpr static bool apply(const TokenizedSrcLine &line,
                              Instr_T &instruction/*,
                              FieldLinkRequest<Reg_T> &linksWithSymbol*/) {
    bool success = false;
    const Token &immToken = line.tokens[tokenIndex];
    ImmConvInfo convInfo;
    Reg_T_S value = getImm(immToken, success, convInfo);

    if (!success) {
      // Could not directly resolve immediate. Register it as a symbol to link
      // to.
      //      linksWithSymbol.field = this;
      //      linksWithSymbol.symbol = immToken;
      //      linksWithSymbol.relocation = immToken.relocation();
      return false;
    }

    if (checkFitsInWidth(value, convInfo, immToken))
      return false;

    ImmParts::apply(value, instruction);
    return true;
  }
  constexpr static bool decode(const Instr_T instruction, const Reg_T address,
                               const ReverseSymbolMap &symbolMap,
                               LineTokens &line) {
    Instr_T reconstructed = 0;
    ImmParts::decodeParts(reconstructed, instruction);
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

    return true;
  }
};

template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts,
          SymbolType symbolType>
using ImmSym =
    ImmBase<tokenIndex, width, repr, ImmParts, symbolType, defaultTransformer>;

template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts>
using Imm = ImmBase<tokenIndex, width, repr, ImmParts, SymbolType::None,
                    defaultTransformer>;

class InstructionBase {
public:
  virtual ~InstructionBase() = default;
  virtual AssembleRes assemble(const TokenizedSrcLine &tokens) = 0;
  virtual Result<LineTokens>
  disassemble(const Instr_T instruction, const Reg_T address,
              const ReverseSymbolMap &symbolMap) const = 0;
  virtual OpPartStruct getOpPart(unsigned partIndex) const = 0;
  virtual const QString &name() const = 0;
  virtual unsigned numOpParts() const = 0;

  /**
   * @brief size
   * @return size of assembled instruction, in bytes.
   */
  unsigned size() const { return m_byteSize; }

  void addExtraMatchCond(const std::function<bool(Instr_T)> &f) {
    m_extraMatchConditions.push_back(f);
  }
  bool hasExtraMatchConds() const { return !m_extraMatchConditions.empty(); }
  bool matchesWithExtras(Instr_T instr) const {
    return llvm::all_of(m_extraMatchConditions,
                        [&](const auto &f) { return f(instr); });
  }

protected:
  /// An optional set of disassembler match conditions, if the default
  /// opcode-based matching is insufficient.
  std::vector<std::function<bool(Instr_T)>> m_extraMatchConditions;
  unsigned m_byteSize = -1;
};

template <typename InstrImpl>
class Instruction : public InstructionBase {
public:
  Instruction() : m_name(InstrImpl::mnemonic()) { verify(); }

  AssembleRes assemble(const TokenizedSrcLine &tokens) override {
    Instr_T instruction = 0;

    InstrImpl::Opcode::Impl::apply(instruction);
    InstrImpl::Fields::Impl::apply(tokens, instruction);

    InstrRes res;
    res.instruction = instruction;
    return res;
  }
  Result<LineTokens>
  disassemble(const Instr_T instruction, const Reg_T address,
              const ReverseSymbolMap &symbolMap) const override {
    LineTokens line;
    line.push_back(name());
    if (!InstrImpl::Fields::Impl::decode(instruction, address, symbolMap,
                                         line)) {
      return Result<LineTokens>(Error(Location(static_cast<int>(address)), ""));
    }
    return Result<LineTokens>(line);
  }
  OpPartStruct getOpPart(unsigned partIndex) const override {
    return InstrImpl::Opcode::Impl::getOpPart(partIndex);
  }
  const QString &name() const override { return m_name; }
  unsigned numOpParts() const override {
    return InstrImpl::Opcode::Impl::numParts();
  }

  /// Verify that the bitranges specified for this operation:
  /// 1. do not overlap
  /// 2. fully defines the instruction (no bits are unaccounted for)
  /// 3. is byte aligned
  /// Using this information, we also set the size of this instruction.
  void verify() {
    std::vector<BitRangeStruct> bitRanges;
    InstrImpl::Opcode::Impl::retrieveBitRanges(bitRanges);
    InstrImpl::Fields::Impl::retrieveBitRanges(bitRanges);

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

private:
  const QString m_name;
};

using InstrMap = std::map<QString, std::shared_ptr<InstructionBase>>;

using InstrVec = std::vector<std::shared_ptr<InstructionBase>>;

} // namespace Ripes
