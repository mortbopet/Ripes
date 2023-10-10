#pragma once

#include <climits>
#include <memory>
#include <string_view>
#include <vector>

#include "isa_defines.h"

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

/// start/stop values for bitranges are inclusive..
template <unsigned _start, unsigned _stop, unsigned _N = 32>
struct BitRange {
  constexpr static unsigned N() { return _N; }
  constexpr static unsigned start() { return _start; }
  constexpr static unsigned stop() { return _stop; }
  constexpr static unsigned width() { return _stop - _start + 1; }
  constexpr static Instr_T getMask() { return generateBitmask(width()); }

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
};

/** @brief OpPart
 * A segment of an operation-identifying field of an instruction.
 */
template <unsigned _value, typename _BitRange>
struct OpPart {
  using BitRange = _BitRange;
  constexpr static unsigned value() { return _value; }

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
struct OpcodeImpl : public OpParts... {
  using BitRanges = BitRangesImpl<typename OpParts::BitRanges...>;
};

template <unsigned tokenIdx, typename _BitRanges>
struct Field {
  using BitRanges = _BitRanges;
};

template <typename... Fields>
struct FieldsImpl : public Fields... {
  using BitRanges = BitRangesImpl<typename Fields::BitRanges...>;
};

/**
 * @brief Reg
 * @param tokenIndex: Index within a list of decoded instruction tokens that
 * corresponds to the register index
 * @param BitRange: range in instruction field containing register index value
 */
template <unsigned tokenIndex, typename BitRange>
struct Reg : public Field<tokenIndex, BitRange> {
  Reg(const QString &_regsd) : regsd(_regsd) {}
  const QString regsd = "reg";
};

template <unsigned _offset, typename _BitRange>
struct ImmPart {
  using BitRange = _BitRange;
};

template <typename... ImmParts>
struct ImmPartsImpl : public ImmParts... {
  using BitRanges = BitRangesImpl<typename ImmParts::BitRange...>;
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
struct ImmBase : public Field<tokenIndex, typename ImmParts::BitRanges> {};

template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts,
          SymbolType symbolType>
using ImmSym =
    ImmBase<tokenIndex, width, repr, ImmParts, symbolType, defaultTransformer>;

template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts>
using Imm = ImmBase<tokenIndex, width, repr, ImmParts, SymbolType::None,
                    defaultTransformer>;

struct InstructionBase {
  virtual ~InstructionBase() = default;
};

template <typename InstrImpl>
struct Instruction : public InstructionBase {};

} // namespace Ripes
