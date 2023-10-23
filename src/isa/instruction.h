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

struct BitRangeBase {
  virtual unsigned n() const = 0;
  virtual unsigned start() const = 0;
  virtual unsigned stop() const = 0;
  virtual unsigned width() const = 0;
  virtual Instr_T getMask() const = 0;
  virtual Instr_T apply(Instr_T value) const = 0;
  virtual Instr_T decode(Instr_T instruction) const = 0;

  bool operator==(const BitRangeBase &other) const {
    return start() == other.start() && stop() == other.stop();
  }
  bool operator<(const BitRangeBase &other) const {
    return (start() == other.start()) ? stop() < other.stop()
                                      : start() < other.start();
  }
};

/// start/stop values for bitranges are inclusive..
template <unsigned _start, unsigned _stop, unsigned _N = 32>
struct BitRange : public BitRangeBase {
  static_assert(isPowerOf2(_N) && "Bitrange N must be power of 2");
  static_assert(_start <= _stop && _stop < _N && "invalid range");

  constexpr static unsigned N() { return _N; }
  constexpr static unsigned Start() { return _start; }
  constexpr static unsigned Stop() { return _stop; }
  constexpr static unsigned Width() { return _stop - _start + 1; }
  constexpr static Instr_T Mask() { return generateBitmask(Width()); }

  constexpr static Instr_T Apply(Instr_T value) {
    return (value & Mask()) << _start;
  }
  constexpr static Instr_T Decode(Instr_T instruction) {
    return (instruction >> _start) & Mask();
  }

  unsigned n() const { return _N; }
  unsigned start() const { return _start; }
  unsigned stop() const { return _stop; }
  unsigned width() const { return Width(); }
  Instr_T getMask() const { return Mask(); }
  Instr_T apply(Instr_T value) const { return Apply(value); }
  Instr_T decode(Instr_T instruction) const { return Decode(instruction); }

  template <typename OtherBitRange>
  constexpr static bool isEqualTo() {
    return _start == OtherBitRange::Start() && _stop == OtherBitRange::Stop();
  }
  template <typename OtherBitRange>
  constexpr static bool isLessThan() {
    return (_start == OtherBitRange::Start()) ? _stop < OtherBitRange::Stop()
                                              : _start < OtherBitRange::Start();
  }
};

template <typename... BitRanges>
struct BitRangesImpl {
  // Combine this type with a set of more BitRanges
  template <typename... OtherBitRanges>
  using CombinedBitRanges = BitRangesImpl<BitRanges..., OtherBitRanges...>;
  // Combine this type with another BitRangesImpl
  template <typename OtherBitRangeImpl>
  using CombineWith =
      typename OtherBitRangeImpl::template CombinedBitRanges<BitRanges...>;

  constexpr static unsigned Width() { return (BitRanges::Width() + ...); }

  constexpr static void Apply(Instr_T &instruction) {
    (BitRanges::Apply(instruction), ...);
  }
  static void
  RetrieveBitRanges(std::vector<std::shared_ptr<BitRangeBase>> &bitRanges) {
    (bitRanges.push_back(std::make_shared<BitRanges>()), ...);
  }

private:
  /// Compile-time verification using recursive templates and static_assert
  template <typename FirstRange, typename... OtherRanges>
  struct Verify {};
  template <typename FirstRange, typename SecondRange, typename... OtherRanges>
  struct Verify<FirstRange, SecondRange, OtherRanges...> {
    /// Returns true if FirstRange and SecondRange are not overlapping
    constexpr static bool IsNotOverlapping() {
      return (FirstRange::Start() > SecondRange::Stop() ||
              FirstRange::Stop() < SecondRange::Start());
    }
    /// Returns true if FirstRange and SecondRange have an equal width
    constexpr static bool HasEqualWidth() {
      return (FirstRange::N() == SecondRange::N());
    }
    enum {
      // Set to true if all BitRanges have been verified
      nonOverlapping = (IsNotOverlapping() &&
                        Verify<FirstRange, OtherRanges...>::nonOverlapping &&
                        Verify<SecondRange, OtherRanges...>::nonOverlapping),
      equalWidth =
          (HasEqualWidth() && Verify<FirstRange, OtherRanges...>::equalWidth &&
           Verify<SecondRange, OtherRanges...>::equalWidth)
    };
  };
  template <typename FirstRange>
  struct Verify<FirstRange> {
    enum { nonOverlapping = true, equalWidth = true };
  };

  static_assert(Verify<BitRanges...>::nonOverlapping,
                "BitRanges overlap with each other");
  static_assert(Verify<BitRanges...>::equalWidth,
                "BitRanges do not have an equal width");

public:
  // NOTE: If a BitRangesImpl type is declared but not used, the verifications
  // may not run.
  // Use this variable in a static assertion in cases where the
  // type may be discarded. See ImmPartsImpl for an example.
  constexpr static bool IsVerified = (Verify<BitRanges...>::nonOverlapping &&
                                      Verify<BitRanges...>::equalWidth);
};

struct OpPartBase;

struct OpPartStruct {
  unsigned value;
  unsigned start, stop, N;
  const OpPartBase *opPart;

  bool operator==(const OpPartStruct &other) const {
    return value == other.value && start == other.start && stop == other.stop;
  }
  bool operator<(const OpPartStruct &other) const {
    if (start == other.start && stop == other.stop)
      return value < other.value;
    return start < other.start;
  }
};

struct OpPartBase {
  virtual unsigned value() const = 0;
  virtual const BitRangeBase &range() const = 0;
  OpPartStruct getStruct() const {
    return OpPartStruct{value(), range().start(), range().stop(), range().n(),
                        this};
  }

  bool operator<(const OpPartBase &other) const {
    if (range() == other.range())
      return value() < other.value();
    return range() < other.range();
  }
  bool matches(Instr_T instruction) const {
    return range().decode(instruction) == value();
  }
};

/** @brief OpPart
 * A segment of an operation-identifying field of an instruction.
 */
template <unsigned _value, typename _BitRange>
class OpPart : public OpPartBase {
public:
  using BitRange = _BitRange;

  unsigned value() const override { return _value; }
  const BitRangeBase &range() const override { return *m_range.get(); }

  constexpr static unsigned Value() { return _value; }

  constexpr static void Apply(Instr_T &instruction) {
    instruction |= BitRange::Apply(Value());
  }
  constexpr static bool Matches(Instr_T instruction) {
    return BitRange::Decode(instruction) == _value;
  }

  template <typename OtherOpPart>
  constexpr static bool IsEqualTo() {
    return _value == OtherOpPart::Value() &&
           BitRange::template IsEqualTo<OtherOpPart>();
  }
  template <typename OtherOpPart>
  constexpr static bool IsLessThan() {
    if (BitRange::template IsEqualTo<OtherOpPart>())
      return _value < OtherOpPart::Value();
    return BitRange::template IsLessThan<OtherOpPart>();
  }

private:
  std::unique_ptr<BitRange> m_range = std::make_unique<BitRange>();

  // Ensure value is not too large to fit in BitRange
  static_assert(isUInt<BitRange::Stop() - BitRange::Start() + 1>(_value),
                "OpPart value is too large to fit in BitRange.");
};

using ResolveSymbolFunc =
    std::function<Result<>(const Location &, Reg_T, Instr_T &, Reg_T)>;

struct FieldLinkRequest {
  ResolveSymbolFunc resolveSymbol;
  QString symbol = QString();
  QString relocation = QString();
};

template <unsigned numParts, typename... OpParts>
static std::array<std::unique_ptr<OpPartBase>, numParts> OP_PARTS = {
    (std::make_unique<OpParts>())...};

template <typename... OpParts>
class OpcodeImpl {
public:
  using BitRanges = BitRangesImpl<typename OpParts::BitRange...>;

  constexpr static void Apply(Instr_T &instruction, FieldLinkRequest &) {
    (OpParts::Apply(instruction), ...);
  }
  constexpr static unsigned NumParts() { return sizeof...(OpParts); }
  constexpr static const OpPartBase *GetOpPart(unsigned partIndex) {
    assert(partIndex < NumParts());

    return OP_PARTS<NumParts(), OpParts...>[partIndex].get();
  }
  constexpr static void
  RetrieveBitRanges(std::vector<std::shared_ptr<BitRangeBase>> &bitRanges) {
    BitRanges::RetrieveBitRanges(bitRanges);
  }
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
struct Field {
  using BitRanges = _BitRanges;
  constexpr static unsigned TokenIndex() { return _tokenIndex; }
};

template <typename... Fields>
class FieldsImpl {
private:
  /// Structs for combining BitRanges from each field
  template <typename... OtherFields>
  struct FieldRanges {};
  template <typename FirstField, typename SecondField, typename... OtherFields>
  struct FieldRanges<FirstField, SecondField, OtherFields...> {
    // Combined BitRanges from all fields
    using BitRanges = typename FirstField::BitRanges::template CombineWith<
        typename SecondField::BitRanges>::
        template CombineWith<typename FieldRanges<OtherFields...>::BitRanges>;
  };
  template <typename FirstField, typename SecondField>
  struct FieldRanges<FirstField, SecondField> {
    // Combined BitRanges from two fields
    using BitRanges = typename FirstField::BitRanges::template CombineWith<
        typename SecondField::BitRanges>;
  };
  template <typename FirstField>
  struct FieldRanges<FirstField> {
    using BitRanges = typename FirstField::BitRanges;
  };

public:
  /// Combined BitRanges from each field
  using BitRanges = typename FieldRanges<Fields...>::BitRanges;

  static Result<> Apply(const TokenizedSrcLine &tokens, Instr_T &instruction,
                        FieldLinkRequest &linksWithSymbol) {
    Result<> res = std::monostate();
    (
        [&] {
          if (auto err = Fields::Apply(tokens, instruction, linksWithSymbol);
              err.isError() && !res.isError()) {
            res = std::get<Error>(err);
          }
        }(),
        ...);
    return res;
  }
  constexpr static bool Decode(const Instr_T instruction, const Reg_T address,
                               const ReverseSymbolMap &symbolMap,
                               LineTokens &line) {
    bool failure = false;
    ((failure |= !Fields::Decode(instruction, address, symbolMap, line)), ...);
    return !failure;
  }
  constexpr static unsigned NumFields() { return sizeof...(Fields); }
  constexpr static void
  RetrieveBitRanges(std::vector<std::shared_ptr<BitRangeBase>> &bitRanges) {
    BitRanges::RetrieveBitRanges(bitRanges);
  }

private:
  // TODO: Verify that:
  // * Registers are not duplicated?? (might be difficult to verify)
  template <typename FirstField, typename... OtherFields>
  struct Verify {};
  template <typename FirstField, typename SecondField, typename... OtherFields>
  struct Verify<FirstField, SecondField, OtherFields...> {
    /// Returns true if SecondField has an index that is directly after
    /// FirstField's index
    constexpr static bool IsInOrder() {
      return (FirstField::TokenIndex() + 1 == SecondField::TokenIndex());
    }
    enum {
      hasSequentialIndices =
          (IsInOrder() &&
           Verify<SecondField, OtherFields...>::hasSequentialIndices),
    };
  };
  template <typename FirstField>
  struct Verify<FirstField> {
    enum { hasSequentialIndices = true };
  };
  template <typename FirstField, typename...>
  struct VerifyFirstIndex {
    enum { indexStartsAtZero = (FirstField::TokenIndex() == 0) };
  };

  static_assert(VerifyFirstIndex<Fields...>::indexStartsAtZero,
                "First field' index is not 0");
  static_assert(Verify<Fields...>::hasSequentialIndices,
                "Fields have duplicate indices");
};

/**
 * @brief Reg
 * @param tokenIndex: Index within a list of decoded instruction tokens that
 * corresponds to the register index
 * @param BitRange: range in instruction field containing register index value
 */
template <unsigned tokenIndex, typename BitRange, typename RegInfo>
struct Reg : public Field<tokenIndex, BitRangesImpl<BitRange>> {
  Reg(const QString &_regsd) : regsd(_regsd) {}

  static Result<> Apply(const TokenizedSrcLine &line, Instr_T &instruction,
                        FieldLinkRequest &) {
    if (tokenIndex + 1 >= line.tokens.size()) {
      // TODO: Make register name static so it can be used in error messages
      //      return Error(line, "Required field '" + regsd + "' not provided"
      //      );
      return Error(line, "Required field index '" +
                             QString::number(tokenIndex) + "' not provided");
    }
    const auto &regToken = line.tokens.at(tokenIndex + 1);
    bool success = false;
    unsigned regIndex = RegInfo::RegNumber(regToken, success);
    if (!success) {
      return Error(line, "Unknown register '" + regToken + "'");
    }
    instruction |= BitRange::Apply(regIndex);
    return std::monostate();
  }
  static bool Decode(const Instr_T instruction, const Reg_T,
                     const ReverseSymbolMap &, LineTokens &line) {
    const unsigned regNumber = BitRange::Decode(instruction);
    const Token registerName(RegInfo::RegName(regNumber));
    if (registerName.isEmpty()) {
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
  // Declaration of BitRanges allows ImmPart to be compatible with ImmPartsImpl
  using BitRanges = BitRangesImpl<BitRange>;

  constexpr static unsigned Offset() { return _offset; }

  constexpr static void Apply(const Instr_T value, Instr_T &instruction) {
    instruction |= BitRange::Apply(value >> _offset);
  }
  constexpr static void Decode(Instr_T &value, const Instr_T instruction) {
    value |= BitRange::Decode(instruction) << _offset;
  }

private:
  static_assert(BitRange::Width() + _offset < BitRange::N(),
                "ImmPart does not fit in BitRange size. Check ImmPart offset"
                " and BitRange width");
};

template <typename... ImmParts>
struct ImmPartsImpl {
  using BitRanges = BitRangesImpl<typename ImmParts::BitRange...>;

  constexpr static void Apply(const Instr_T value, Instr_T &instruction) {
    (ImmParts::Apply(value, instruction), ...);
  }
  constexpr static void Decode(Instr_T &value, const Instr_T instruction) {
    (ImmParts::Decode(value, instruction), ...);
  }

private:
  template <typename FirstPart, typename... OtherParts>
  struct Verify {};
  template <typename FirstPart, typename SecondPart, typename... OtherParts>
  struct Verify<FirstPart, SecondPart, OtherParts...> {
    /// Returns true if FirstPart and SecondPart are not overlapping
    constexpr static bool IsNotOverlapping() {
      return (FirstPart::Offset() >
                  (SecondPart::Offset() + SecondPart::BitRange::Width()) ||
              SecondPart::Offset() >
                  (FirstPart::Offset() + FirstPart::BitRange::Width()));
    }
    enum {
      nonOverlapping = (IsNotOverlapping() &&
                        Verify<FirstPart, OtherParts...>::nonOverlapping &&
                        Verify<SecondPart, OtherParts...>::nonOverlapping)
    };
  };
  template <typename FirstPart>
  struct Verify<FirstPart> {
    enum { nonOverlapping = true };
  };

  static_assert(Verify<ImmParts...>::nonOverlapping,
                "Combined ImmParts overlap with each other");
  static_assert(BitRanges::IsVerified, "Could not verify ImmParts BitRanges");
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
// TODO: Maybe remove width as a template parameter since it can be retrieved
// from ImmParts::BitRanges::Width()
template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts,
          SymbolType symbolType, SymbolTransformer transformer>
struct ImmBase : public Field<tokenIndex, typename ImmParts::BitRanges> {
  static_assert(
      width == ImmParts::BitRanges::Width(),
      "Immediate's width does not match the combined width of its BitRanges");

  using Reg_T_S = typename std::make_signed<Reg_T>::type;
  using Reg_T_U = typename std::make_unsigned<Reg_T>::type;

  constexpr static int64_t GetImm(const QString &immToken, bool &success,
                                  ImmConvInfo &convInfo) {
    return repr == Repr::Signed
               ? getImmediateSext32(immToken, success, &convInfo)
               : getImmediate(immToken, success, &convInfo);
  }

  static Result<> CheckFitsInWidth(Reg_T_S value, const Location &sourceLine,
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

  static Result<> ApplySymbolResolution(const Location &loc, Reg_T symbolValue,
                                        Instr_T &instruction, Reg_T address) {
    ImmConvInfo convInfo;
    convInfo.radix = reprToRadix(repr);
    Reg_T adjustedValue = symbolValue;
    if (symbolType == SymbolType::Relative)
      adjustedValue -= address;

    adjustedValue = transformer(adjustedValue);

    if (auto res = CheckFitsInWidth(adjustedValue, loc, convInfo);
        res.isError())
      return res.error();

    ImmParts::Apply(adjustedValue, instruction);

    return Result<>::def();
  }

  static Result<> Apply(const TokenizedSrcLine &line, Instr_T &instruction,
                        FieldLinkRequest &linksWithSymbol) {
    if (tokenIndex + 1 >= line.tokens.size()) {
      return Error(line, "Required immediate with field index '" +
                             QString::number(tokenIndex) + "' not provided");
    }
    bool success = false;
    const Token &immToken = line.tokens[tokenIndex + 1];
    ImmConvInfo convInfo;
    Reg_T_S value = GetImm(immToken, success, convInfo);

    if (!success) {
      // Could not directly resolve immediate. Register it as a symbol to link
      // to.
      linksWithSymbol.resolveSymbol = ApplySymbolResolution;
      linksWithSymbol.symbol = immToken;
      linksWithSymbol.relocation = immToken.relocation();
      return Error(line, "Could not resolve immediate");
    }

    if (auto res = CheckFitsInWidth(value, line, convInfo, immToken);
        res.isError())
      return res.error();

    ImmParts::Apply(value, instruction);
    return std::monostate();
  }
  constexpr static bool Decode(const Instr_T instruction, const Reg_T address,
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
  virtual const OpPartBase *getOpPart(unsigned partIndex) const = 0;
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
  // TODO: Assertions
  // * Opcode bitranges should not overlap with Fields bitranges
  // * No bits are unaccounted for
  // * Instruction is byte aligned
  struct Verify {
    using BitRanges =
        typename InstrImpl::Opcode::Impl::BitRanges::template CombineWith<
            InstrImpl::Fields::Impl::BitRanges>;
    static_assert(InstrImpl::Opcode::Impl::BitRanges::template CombineWith<
                      InstrImpl::Fields::Impl::BitRanges>::IsVerified,
                  "Could not verify combined bitranges from Opcode and Fields");
  };

  Instruction() : m_name(InstrImpl::mnemonic()) { verify(); }

  AssembleRes assemble(const TokenizedSrcLine &tokens) override {
    Instr_T instruction = 0;
    FieldLinkRequest linksWithSymbol;

    InstrImpl::Opcode::Impl::Apply(instruction, linksWithSymbol);
    if (auto fieldRes = InstrImpl::Fields::Impl::Apply(tokens, instruction,
                                                       linksWithSymbol);
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
    if (!InstrImpl::Fields::Impl::Decode(instruction, address, symbolMap,
                                         line)) {
      return Error(Location(static_cast<int>(address)), "");
    }
    return line;
  }
  const OpPartBase *getOpPart(unsigned partIndex) const override {
    return InstrImpl::Opcode::Impl::GetOpPart(partIndex);
  }
  const QString &name() const override { return m_name; }
  unsigned numOpParts() const override {
    return InstrImpl::Opcode::Impl::NumParts();
  }

  /// Verify that the bitranges specified for this operation:
  /// 1. do not overlap
  /// 2. fully defines the instruction (no bits are unaccounted for)
  /// 3. is byte aligned
  /// Using this information, we also set the size of this instruction.
  void verify() {
    std::vector<std::shared_ptr<BitRangeBase>> bitRanges;
    InstrImpl::Opcode::Impl::RetrieveBitRanges(bitRanges);
    InstrImpl::Fields::Impl::RetrieveBitRanges(bitRanges);

    // 1.
    std::set<unsigned> registeredBits;
    for (auto &range : bitRanges) {
      for (unsigned i = range->start(); i <= range->stop(); ++i) {
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
