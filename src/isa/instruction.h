#pragma once

#include <climits>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>

#include <QList>

#include "../binutils.h"
#include "isa_defines.h"
#include <STLExtras.h>

namespace Ripes {

/** @brief Ensure that the template parameters for BitRangesImpl are always the
 * BitRange type.
 * Used in static assertions.
 * @param BitRange...: Types that might be BitRange
 */
template <typename MaybeBitRange>
using IsBitRange =
    decltype((MaybeBitRange::N + MaybeBitRange::start + MaybeBitRange::stop),
             MaybeBitRange::getInstance(),
             std::declval<MaybeBitRange>().N +
                 std::declval<MaybeBitRange>().start +
                 std::declval<MaybeBitRange>().stop +
                 std::declval<MaybeBitRange>().width(),
             std::declval<MaybeBitRange>().apply(
                 std::declval<MaybeBitRange>().getMask()),
             std::declval<MaybeBitRange>().decode(
                 std::declval<MaybeBitRange>().getMask()));

template <typename MaybeOpPart>
using IsOpPart =
    decltype(MaybeOpPart::value, MaybeOpPart::getInstance(),
             std::declval<MaybeOpPart>().apply(std::declval<Instr_T &>()),
             std::declval<MaybeOpPart>().matches(std::declval<Instr_T>()),
             std::declval<MaybeOpPart>().value,
             std::declval<MaybeOpPart>().range,
             std::declval<MaybeOpPart>() == std::declval<MaybeOpPart>(),
             std::declval<MaybeOpPart>() < std::declval<MaybeOpPart>());

struct FieldLinkRequest;

template <typename MaybeField>
using IsField =
    decltype(MaybeField::apply(std::declval<const TokenizedSrcLine &>(),
                               std::declval<Instr_T &>(),
                               std::declval<FieldLinkRequest &>()),
             MaybeField::decode(std::declval<const Instr_T>(),
                                std::declval<const Reg_T>(),
                                std::declval<const ReverseSymbolMap &>(),
                                std::declval<LineTokens &>()));

template <typename MaybeBitRange>
constexpr bool VerifyBitRange = qxp::is_detected_v<IsBitRange, MaybeBitRange>;
template <typename MaybeField>
constexpr bool VerifyField = qxp::is_detected_v<IsField, MaybeField>;

template <template <typename...> class Op, typename...>
struct VerifyValidTypes {};
template <template <typename...> class Op, typename Type, typename... NextTypes>
struct VerifyValidTypes<Op, Type, NextTypes...> {
  static_assert(qxp::is_detected_v<Op, Type>, "Invalid type");

  constexpr static VerifyValidTypes<Op, NextTypes...> VerifyRest{};
};

/** @brief A range of bits determined at run-time
 *  Usually created from BitRange::getInstance()
 */
struct BitRangeBase {
  constexpr BitRangeBase(unsigned _start, unsigned _stop, unsigned _N)
      : start(_start), stop(_stop), N(_N) {}

  const unsigned start, stop, N;

  constexpr unsigned width() const { return stop - start + 1; }
  constexpr Instr_T getMask() const { return vsrtl::generateBitmask(width()); }
  constexpr Instr_T apply(Instr_T value) const {
    return (value & getMask()) << start;
  }
  constexpr Instr_T decode(Instr_T instruction) const {
    return (instruction >> start) & getMask();
  }

  constexpr bool operator==(const BitRangeBase &other) const {
    return N == other.N && start == other.start && stop == other.stop;
  }
  constexpr bool operator<(const BitRangeBase &other) const {
    if (N != other.N)
      return (N < other.N);
    return (start == other.start) ? stop < other.stop : start < other.start;
  }
};

/** @brief A range of bits determined at compile-time
 * NOTE: start/stop values for bitranges are inclusive..
 * @param _start: Starting index of the range
 * @param _stop: Ending index of the range (inclusive)
 * @param _N: Number of bits in the range (should be power of 2)
 */
template <unsigned _start, unsigned _stop, unsigned _N = 32>
struct BitRange : public BitRangeBase {
  static_assert(isPowerOf2(_N), "Bitrange N must be power of 2");
  static_assert(_start <= _stop && _stop < _N, "invalid range");

  constexpr BitRange() : BitRangeBase(_start, _stop, _N) {}

  constexpr static unsigned start = _start;
  constexpr static unsigned stop = _stop;
  constexpr static unsigned N = _N;

  constexpr static BitRange getInstance() { return BitRange(); }
};

/** A set of BitRanges.
 * @param BitRanges: A set of BitRange types. All BitRanges must not overlap.
 */
template <typename... BitRanges>
struct BitRangeSet {
  /// Combine this type with a set of more BitRanges
  template <typename... OtherBitRanges>
  using CombinedBitRanges = BitRangeSet<BitRanges..., OtherBitRanges...>;

  /// Combine this type with another BitRangesImpl
  template <typename OtherBitRangeImpl>
  using CombineWith =
      typename OtherBitRangeImpl::template CombinedBitRanges<BitRanges...>;

  /// Returns the combined width of all BitRanges
  constexpr static unsigned width() {
    return (BitRanges::getInstance().width() + ... + 0);
  }

private:
  /// Compile-time verification using recursive templates and static_assert
  template <typename...>
  struct Verify {};
  template <typename FirstRange, typename SecondRange, typename... OtherRanges>
  struct Verify<FirstRange, SecondRange, OtherRanges...> {
    /// Assert that all BitRanges do not overlap with each other
    static_assert((FirstRange::start > SecondRange::stop ||
                   FirstRange::stop < SecondRange::start),
                  "BitRanges overlap with each other");

    /// Assert that all BitRanges have equal sizes
    static_assert((FirstRange::N == SecondRange::N),
                  "BitRanges do not have equal sizes");

    /// Verify all combinations of ranges to ensure they don't overlap
    constexpr static Verify<FirstRange, OtherRanges...> nextVerify0{};
    constexpr static Verify<SecondRange, OtherRanges...> nextVerify1{};
  };

  /// Ensure that no ranges in the set overlap with each other
  constexpr static VerifyValidTypes<IsBitRange, BitRanges...> verifyTypes{};
  /// Ensure all template parameters are subtypes of BitRange
  constexpr static Verify<BitRanges...> verifyAll{};
};

/** @brief A segment of an operation-identifying field of an instruction created
 * at run-time.
 * Usually created by OpPart::getInstance()
 */
struct OpPartBase {
  constexpr OpPartBase(unsigned _value, BitRangeBase _range)
      : value(_value), range(_range) {}

  const unsigned value;
  const BitRangeBase range;

  constexpr bool operator==(const OpPartBase &other) const {
    return value == other.value && range == other.range;
  }
  constexpr bool operator<(const OpPartBase &other) const {
    if (range == other.range)
      return value < other.value;
    return range < other.range;
  }

  /// Applies this OpPart's encoding to the instruction.
  constexpr void apply(Instr_T &instruction) const {
    instruction |= range.apply(value);
  }
  /// Returns true if this OpPart is contained in the instruction.
  constexpr bool matches(Instr_T instruction) const {
    return range.decode(instruction) == value;
  }
};

/** @brief A segment of an operation-identifying field of an instruction created
 * at compile-time.
 * @param _value: The value that identifies this OpPart.
 * @param _BitRange: The range of bits that contain this OpPart. Must be a
 * BitRange type.
 */
template <unsigned _value, typename _BitRange>
class OpPart : public OpPartBase {
public:
  using BitRange = _BitRange;

  constexpr static unsigned value = _value;

  static_assert(isUInt<BitRange::getInstance().width()>(value),
                "OpPart value is too large to fit in BitRange.");
  static_assert(VerifyBitRange<BitRange>,
                "OpPart can only contain a BitRange type");

  constexpr OpPart() : OpPartBase(value, BitRange::getInstance()) {}

  constexpr static OpPart getInstance() { return OpPart(); }
};

/** @brief An OpPart for declaring BitRanges of unused zeros */
template <unsigned start, unsigned stop, unsigned N = 32>
struct OpPartZeroes : public OpPart<0, BitRange<start, stop, N>> {};

/** @brief Function type for resolving symbols. */
using ResolveSymbolFunc =
    std::function<Result<>(const Location &, Reg_T, Instr_T &, Reg_T)>;

/** @brief A request to link a field of an instruction to a symbol. */
struct FieldLinkRequest {
  ResolveSymbolFunc resolveSymbol;
  QString symbol = QString();
  QString relocation = QString();
};

/// Struct used to dynamically index OpParts
template <typename OpPart, typename... NextOpParts>
struct IndexedOpPart {
  constexpr static OpPartBase getOpPart(unsigned partIndex) {
    assert(partIndex < sizeof...(NextOpParts) + 1 &&
           "OpPart index out of range");
    if (partIndex == 0) {
      // OpPart found
      return OpPart::getInstance();
    } else if constexpr (sizeof...(NextOpParts) > 0) {
      // Check next OpPart
      return IndexedOpPart<NextOpParts...>::getOpPart(partIndex - 1);
    }

    // The assertion above should make this line unreachable
    Q_UNREACHABLE();
  }
};

/** @brief A set of OpParts that identifies an instruction.
 * @param OpParts: A set of OpPart types. All OpParts' BitRanges must not
 * overlap.
 */
template <typename... OpParts>
struct OpcodeSet {
  using BitRanges = BitRangeSet<typename OpParts::BitRange...>;

  static_assert(sizeof...(OpParts) > 0, "No opcode provided");

  /// Applies each OpPart's encoding to the instruction.
  constexpr static void apply(Instr_T &instruction, FieldLinkRequest &) {
    (OpParts::getInstance().apply(instruction), ...);
  }
  /// Returns the number of OpParts in this opcode.
  constexpr static unsigned numParts() { return sizeof...(OpParts); }
  /// Returns a pointer to a dynamically accessible OpPart. (needed for the
  /// assembly matcher)
  constexpr static OpPartBase getOpPart(unsigned partIndex) {
    assert(partIndex < numParts() && "OpPart index out of range");

    return IndexedOpPart<OpParts...>::getOpPart(partIndex);
  }

private:
  /// Run verifications for all BitRanges
  constexpr static BitRanges ranges{};
  /// Verify all template parameters are subtypes of OpPart
  constexpr static VerifyValidTypes<IsOpPart, OpParts...> verifyTypes{};
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

/** @brief An instruction field defined at compile-time.
 * @param _tokenIndex: The index of this field in an assembly instruction
 * (starting at 0).
 * @param _BitRanges: A set of BitRanges that define which bits contain field in
 * the instruction. Must be BitRangesImpl type.
 */
template <unsigned _tokenIndex, typename _BitRanges>
struct Field {
  using BitRanges = _BitRanges;

private:
  /// Run verifications for all BitRanges
  constexpr static BitRanges ranges{};
};

/** @brief A set of fields for an instruction
 * @param Fields: The set of Field types. Field BitRanges must not overlap.
 * Fields must have sequential indices, starting at 0.
 */
template <template <unsigned> typename... Fields>
class FieldSet {
private:
  /// Structs used to combine BitFields and assign token indices
  template <unsigned, template <unsigned> typename...>
  struct IndexedFieldSet {
    using BitRanges = BitRangeSet<>;
  };
  template <unsigned TokenIndex, template <unsigned> typename FirstField,
            template <unsigned> typename... NextFields>
  struct IndexedFieldSet<TokenIndex, FirstField, NextFields...> {
    using IndexedField = FirstField<TokenIndex>;
    using NextIndexedFieldSet = IndexedFieldSet<TokenIndex + 1, NextFields...>;
    using BitRanges = typename IndexedField::BitRanges::template CombineWith<
        typename NextIndexedFieldSet::BitRanges>;

    static_assert(VerifyField<IndexedField>, "Invalid Field type");

    static Result<> apply(const TokenizedSrcLine &tokens, Instr_T &instruction,
                          FieldLinkRequest &linksWithSymbol) {
      // Apply this field
      if (auto err = IndexedField::apply(tokens, instruction, linksWithSymbol);
          err.isError()) {
        return err;
      }
      // Apply further fields if they exist
      if constexpr (sizeof...(NextFields) > 0)
        return NextIndexedFieldSet::apply(tokens, instruction, linksWithSymbol);
      else
        return Result<>::def();
    }
    constexpr static bool decode(const Instr_T instruction, const Reg_T address,
                                 const ReverseSymbolMap &symbolMap,
                                 LineTokens &line) {
      // Decode this field
      if (!IndexedField::decode(instruction, address, symbolMap, line)) {
        return false;
      }
      // Decode further fields if they exist
      if constexpr (sizeof...(NextFields) > 0)
        return NextIndexedFieldSet::decode(instruction, address, symbolMap,
                                           line);
      else
        return true;
    }
  };

  using IndexedFields = IndexedFieldSet<0, Fields...>;

public:
  /// Combined BitRanges from each field in the set
  using BitRanges = typename IndexedFields::BitRanges;

  /// Applies each Field's encoding to the instruction.
  static Result<> apply(const TokenizedSrcLine &tokens, Instr_T &instruction,
                        FieldLinkRequest &linksWithSymbol) {
    if constexpr (sizeof...(Fields) > 0)
      return IndexedFields::apply(tokens, instruction, linksWithSymbol);
    else
      return Result<>::def();
  }
  /// Decodes each field into an assembly instruction line.
  constexpr static bool decode(const Instr_T instruction, const Reg_T address,
                               const ReverseSymbolMap &symbolMap,
                               LineTokens &line) {
    if constexpr (sizeof...(Fields) > 0)
      return IndexedFields::decode(instruction, address, symbolMap, line);
    else
      return true;
  }

  /// Returns the number of Fields in this set.
  constexpr static unsigned numFields() { return sizeof...(Fields); }

private:
  /// This calls all BitRanges' static assertions
  constexpr static BitRanges Ranges{};
};

/**
 * @brief Reg
 * @param RegImpl: A type that declares `constexpr static std::string_view
 * Name`. This defines the name of this register.
 * @param tokenIndex: Index within a list of decoded instruction tokens that
 * corresponds to the register index
 * @param BitRange: range in instruction field containing register index value
 * @param RegInfo: A type that declares 2 functions:
 * `static unsigned int RegNumber(const QString &reg, bool &success)` and
 * `static QString RegName(unsigned i)`
 */
template <typename RegImpl, unsigned tokenIndex, typename BitRange,
          typename RegInfo>
struct Reg : public Field<tokenIndex, BitRangeSet<BitRange>> {
  static_assert(VerifyBitRange<BitRange>, "Invalid BitRange type");

  Reg() : regsd(RegImpl::Name.data()) {}

  /// Applies this register's encoding to the instruction.
  static Result<> apply(const TokenizedSrcLine &line, Instr_T &instruction,
                        FieldLinkRequest &) {
    if (tokenIndex + 1 >= line.tokens.size()) {
      return Error(line, "Required field '" + QString(RegImpl::Name.data()) +
                             "' (index " + QString::number(tokenIndex) +
                             ") not provided");
    }
    const auto &regToken = line.tokens.at(tokenIndex + 1);
    bool success = false;
    unsigned regIndex = RegInfo::RegNumber(regToken, success);
    if (!success) {
      return Error(line, "Unknown register '" + regToken + "'");
    }
    instruction |= BitRange::getInstance().apply(regIndex);
    return Result<>::def();
  }
  /// Decodes this register into its name. Adds it to the assembly line.
  static bool decode(const Instr_T instruction, const Reg_T,
                     const ReverseSymbolMap &, LineTokens &line) {
    const unsigned regNumber = BitRange::getInstance().decode(instruction);
    const Token registerName(RegInfo::RegName(regNumber));
    if (registerName.isEmpty()) {
      return false;
    }
    line.push_back(registerName);
    return true;
  }

  const QString regsd = "reg";
};

/** @brief A part of an immediate field.
 * @param _offset: The offset applied to this part when it is constructed into
 * an immediate value.
 * @param _BitRange: The range of bits that contain this part.
 */
template <unsigned _offset, typename _BitRange>
struct ImmPartBase {
  using BitRange = _BitRange;
  // Declaration of BitRanges allows ImmPart to be compatible with
  // ImmPartsImpl
  using BitRanges = BitRangeSet<BitRange>;

  // TODO(raccog): Assert that the BitRange's width added to the offset is not
  // larger than the ISA's register width

  /// Returns the offset applied to this part when it is constructed into an
  /// immediate value.
  constexpr static unsigned Offset() { return _offset; }

  /// Applies this immediate part's encoding to the instruction.
  constexpr static void Apply(const Instr_T value, Instr_T &instruction) {
    instruction |= BitRange::getInstance().apply(value >> _offset);
  }
  /// Decodes this immediate part into its value, combining it with other
  /// values.
  constexpr static void Decode(Instr_T &value, const Instr_T instruction) {
    value |= BitRange::getInstance().decode(instruction) << _offset;
  }
};

template <unsigned offset, unsigned start, unsigned stop, unsigned N = 32>
using ImmPart = ImmPartBase<offset, BitRange<start, stop, N>>;

/** @brief A set of immediate parts for an immediate field.
 * @param ImmParts: The set of ImmPart types. ImmPart BitRanges must not
 * overlap. ImmParts must not overlap when constructed into an immediate value
 * with their offsets applied.
 */
template <typename... ImmParts>
struct ImmPartsImpl {
  using BitRanges = BitRangeSet<typename ImmParts::BitRange...>;

  /// Applies each immediate part's encoding to the instruction.
  constexpr static void Apply(const Instr_T value, Instr_T &instruction) {
    (ImmParts::Apply(value, instruction), ...);
  }
  /// Decodes this immediate into its value by combining values from each part.
  constexpr static void Decode(Instr_T &value, const Instr_T instruction) {
    (ImmParts::Decode(value, instruction), ...);
  }

private:
  /// Verify that each immediate does not overlap.
  template <typename FirstPart, typename... OtherParts>
  struct Verify {};
  template <typename FirstPart, typename SecondPart, typename... OtherParts>
  struct Verify<FirstPart, SecondPart, OtherParts...> {
    /// Asserts that FirstPart and SecondPart are not overlapping
    static_assert(
        (FirstPart::Offset() >= (SecondPart::Offset() +
                                 SecondPart::BitRange::getInstance().width()) ||
         SecondPart::Offset() >= (FirstPart::Offset() +
                                  FirstPart::BitRange::getInstance().width())),
        "Immediate has parts with overlapping offsets");

    constexpr static Verify<FirstPart, OtherParts...> Verify0{};
    constexpr static Verify<SecondPart, OtherParts...> Verify1{};
  };

  constexpr static Verify<ImmParts...> VerifyAll{};
};

enum class Repr { Unsigned, Signed, Hex };
enum class SymbolType { None, Relative, Absolute };

static inline Radix reprToRadix(Repr repr) {
  if (repr == Repr::Unsigned)
    return Radix::Unsigned;
  if (repr == Repr::Signed)
    return Radix::Signed;
  if (repr == Repr::Hex)
    return Radix::Hex;
  return Radix::Unsigned;
}

typedef Reg_T (*SymbolTransformer)(Reg_T);

/// The default immediate transformer. Returns the value unchanged.
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
 * @param ImmParts: A set of ImmPart types that define the encoding of this
 * immediate field.
 */
template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts,
          SymbolType symbolType, SymbolTransformer transformer>
struct ImmBase : public Field<tokenIndex, typename ImmParts::BitRanges> {
  static_assert(width >= ImmParts::BitRanges::width(),
                "An immediate's combined parts are larger than its width");

  using Reg_T_S = typename std::make_signed<Reg_T>::type;
  using Reg_T_U = typename std::make_unsigned<Reg_T>::type;

  /// Converts a string to its immediate value (if it exists). Success is set to
  /// false if this fails.
  constexpr static int64_t getImm(const QString &immToken, bool &success,
                                  ImmConvInfo &convInfo) {
    return repr == Repr::Signed
               ? getImmediateSext32(immToken, success, &convInfo)
               : getImmediate(immToken, success, &convInfo);
  }

  /// Returns an error if `value` does not fit in this immediate.
  static Result<> checkFitsInWidth(Reg_T_S value, const Location &sourceLine,
                                   ImmConvInfo &convInfo,
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
      // immediate check). e.g. a signed immediate value of 12 bits must be
      // able to accept 0xAFF.
      bool isBitwize =
          convInfo.radix == Radix::Hex || convInfo.radix == Radix::Binary;
      if (isBitwize) {
        err = !isUInt(width, value);
      }

      if (!isBitwize || (isBitwize && err)) {
        // A signed representation using an integer value in assembly OR a
        // negative bitwize value which is represented in its full length
        // (e.g. 0xFFFFFFF1).
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

  /// Symbol resolver function for this immediate.
  static Result<> applySymbolResolution(const Location &loc, Reg_T symbolValue,
                                        Instr_T &instruction, Reg_T address) {
    ImmConvInfo convInfo;
    convInfo.radix = reprToRadix(repr);
    Reg_T adjustedValue = symbolValue;
    if (symbolType == SymbolType::Relative)
      adjustedValue -= address;

    adjustedValue = transformer(adjustedValue);

    if (auto res = checkFitsInWidth(adjustedValue, loc, convInfo);
        res.isError())
      return res.error();

    ImmParts::Apply(adjustedValue, instruction);

    return Result<>::def();
  }

  /// Applies this immediate's encoding to the instruction.
  static Result<> apply(const TokenizedSrcLine &line, Instr_T &instruction,
                        FieldLinkRequest &linksWithSymbol) {
    if (tokenIndex + 1 >= line.tokens.size()) {
      return Error(line, "Required immediate with field index '" +
                             QString::number(tokenIndex) + "' not provided");
    }
    bool success = false;
    const Token &immToken = line.tokens[tokenIndex + 1];
    ImmConvInfo convInfo;
    Reg_T_S value = getImm(immToken, success, convInfo);

    if (!success) {
      // Could not directly resolve immediate. Register it as a symbol to link
      // to.
      linksWithSymbol.resolveSymbol = applySymbolResolution;
      linksWithSymbol.symbol = immToken;
      linksWithSymbol.relocation = immToken.relocation();
      return Result<>::def();
    }

    if (auto res = checkFitsInWidth(value, line, convInfo, immToken);
        res.isError())
      return res.error();

    ImmParts::Apply(value, instruction);
    return Result<>::def();
  }
  /// Decodes this immediate part into its value, adding it to the assembly
  /// line.
  constexpr static bool decode(const Instr_T instruction, const Reg_T address,
                               const ReverseSymbolMap &symbolMap,
                               LineTokens &line) {
    Instr_T reconstructed = 0;
    ImmParts::Decode(reconstructed, instruction);
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

/** @brief Shorthand for an Immediate with the default value transformer */
template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts,
          SymbolType symbolType>
using ImmSym =
    ImmBase<tokenIndex, width, repr, ImmParts, symbolType, defaultTransformer>;

/** @brief Shorthand for an Immediate with the default value transformer and no
 * symbol type. */
template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts>
using Imm = ImmBase<tokenIndex, width, repr, ImmParts, SymbolType::None,
                    defaultTransformer>;

/** @brief A no-template, abstract class that defines an instruction. */
class InstructionBase {
public:
  InstructionBase(unsigned byteSize) : m_byteSize(byteSize) {}
  virtual ~InstructionBase() = default;
  /// Assembles a line of tokens into an encoded program.
  virtual AssembleRes assemble(const TokenizedSrcLine &tokens) = 0;
  /// Disassembles an encoded program into a tokenized assembly program.
  virtual Result<LineTokens>
  disassemble(const Instr_T instruction, const Reg_T address,
              const ReverseSymbolMap &symbolMap) const = 0;
  /// Returns a pointer to a dynamically accessible OpPart. (needed for the
  /// assembly matcher)
  virtual OpPartBase getOpPart(unsigned partIndex) const = 0;
  /// Returns the name of this instruction.
  virtual const QString &name() const = 0;
  /// Returns the number of OpParts in this instruction.
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
  unsigned m_byteSize;
};

/** @brief Asserts that this instruction has no overlapping fields, has all
 * bits utilized, and is byte-aligned.
 * @param InstrImpl: The instruction type to assert.
 */
template <typename InstrImpl>
struct InstrVerify : std::true_type {
  using BitRanges = typename InstrImpl::Opcode::BitRanges::template CombineWith<
      typename InstrImpl::Fields::BitRanges>;

  static_assert(BitRanges::width() == InstrImpl::InstrBits(),
                "Instruction does not utilize all bits");
  static_assert((BitRanges::width() % CHAR_BIT == 0),
                "Instruction width is not byte aligned");
};

template <typename InstrImpl>
struct InstrByteSize {
  using BitRanges = typename InstrImpl::Opcode::BitRanges::template CombineWith<
      typename InstrImpl::Fields::BitRanges>;

  constexpr static unsigned ByteSize = BitRanges::width() / CHAR_BIT;
};

/** @brief An ISA instruction defined at compile-time.
 * @param InstrImpl: The type defining a single instruction. Must define the
 * following:
 *
 * `struct Opcode : public OpPartSet;`
 * `struct Fields : public FieldSet;`
 * `constexpr static std::string_view Name`
 */
template <typename InstrImpl>
struct Instruction : public InstructionBase {
  Instruction()
      : InstructionBase(InstrByteSize<InstrImpl>::ByteSize),
        m_name(InstrImpl::Name.data()) {}

  AssembleRes assemble(const TokenizedSrcLine &tokens) override {
    Instr_T instruction = 0;
    FieldLinkRequest linksWithSymbol;

    InstrImpl::Opcode::apply(instruction, linksWithSymbol);
    if (auto fieldRes =
            InstrImpl::Fields::apply(tokens, instruction, linksWithSymbol);
        fieldRes.isError()) {
      return std::get<Error>(fieldRes);
    }

    InstrRes res;
    res.linksWithSymbol = linksWithSymbol;
    res.instruction = instruction;
    return res;
  }
  Result<LineTokens>
  disassemble(const Instr_T instruction, const Reg_T address,
              const ReverseSymbolMap &symbolMap) const override {
    LineTokens line;
    line.push_back(name());
    if (!InstrImpl::Fields::decode(instruction, address, symbolMap, line)) {
      return Error(Location(static_cast<int>(address)), "");
    }
    return line;
  }
  OpPartBase getOpPart(unsigned partIndex) const override {
    return InstrImpl::Opcode::getOpPart(partIndex);
  }
  const QString &name() const override { return m_name; }
  unsigned numOpParts() const override { return InstrImpl::Opcode::numParts(); }

private:
  const QString m_name;
};

using InstrMap = std::map<QString, std::shared_ptr<InstructionBase>>;

using InstrVec = std::vector<std::shared_ptr<InstructionBase>>;

template <typename InstrVecType, typename... Instructions>
constexpr inline static void _enableInstructions(InstrVecType &instructions) {
  (instructions.emplace_back(std::make_unique<Instructions>()), ...);
}

template <typename... Instructions>
constexpr inline static void enableInstructions(InstrVec &instructions) {
  // TODO(raccog): Combine instructions into a struct that can be used for
  // compile-time validation
  // TODO(raccog): Ensure no duplicate instruction definitions (will be
  // difficult, since enableInstructions can be called multiple times)
  return _enableInstructions<InstrVec, Instructions...>(instructions);
}

} // namespace Ripes
