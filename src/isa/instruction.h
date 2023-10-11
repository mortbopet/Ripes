#pragma once

#include <climits>
#include <memory>
#include <string_view>
#include <vector>

#include <QList>

#include "binutils.h"
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
struct OpcodeImpl : public OpParts... {
  using BitRanges = BitRangesImpl<typename OpParts::BitRange...>;

  constexpr static void apply(Instr_T &instruction) {
    (OpParts::apply(instruction), ...);
  }
};

template <typename Field>
struct FieldLinkRequest {
  Field const *field = nullptr;
  QString symbol = QString();
  QString relocation = QString();
};

template <unsigned tokenIdx, typename _BitRanges>
struct Field {
  using BitRanges = _BitRanges;
};

template <typename... Fields>
struct FieldsImpl : public Fields... {
  using BitRanges = BitRangesImpl<typename Fields::BitRanges...>;

  constexpr static void apply(const TokenizedSrcLine &tokens,
                              Instr_T &instruction) {
    (Fields::apply(tokens, instruction), ...);
  }
};

namespace RVISA {
unsigned regNumber(const QString &regToken, bool &success);
}

/**
 * @brief Reg
 * @param tokenIndex: Index within a list of decoded instruction tokens that
 * corresponds to the register index
 * @param BitRange: range in instruction field containing register index value
 */
template <unsigned tokenIndex, typename BitRange>
struct Reg : public Field<tokenIndex, BitRange> {
  Reg(const QString &_regsd) : regsd(_regsd) {}

  constexpr static bool
  apply(const TokenizedSrcLine &line,
        Instr_T &instruction /*, FieldLinkRequest<Reg_T> &*/) {
    const auto &regToken = line.tokens.at(tokenIndex);
    bool success = false;
    unsigned regIndex = RVISA::regNumber(regToken, success);
    if (!success) {
      // TODO: Set error in FieldLinkRequest
      //      return Error(line, "Unknown register '" + regToken + "'");
      return false;
    }
    instruction |= BitRange::apply(regIndex);
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
};

template <typename... ImmParts>
struct ImmPartsImpl : public ImmParts... {
  using BitRanges = BitRangesImpl<typename ImmParts::BitRange...>;

  constexpr static void apply(const Instr_T value, Instr_T &instruction) {
    (ImmParts::apply(value, instruction), ...);
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
};

template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts,
          SymbolType symbolType>
using ImmSym =
    ImmBase<tokenIndex, width, repr, ImmParts, symbolType, defaultTransformer>;

template <unsigned tokenIndex, unsigned width, Repr repr, typename ImmParts>
using Imm = ImmBase<tokenIndex, width, repr, ImmParts, SymbolType::None,
                    defaultTransformer>;

struct InstructionBase {
  virtual ~InstructionBase() = default;
  virtual Instr_T assemble(const TokenizedSrcLine &tokens) = 0;
};

template <typename InstrImpl>
struct Instruction : public InstructionBase {
  Instr_T assemble(const TokenizedSrcLine &tokens) override {
    Instr_T instruction = 0;

    InstrImpl::Opcode::Impl::apply(instruction);
    InstrImpl::Fields::Impl::apply(tokens, instruction);

    return instruction;
  }
};

} // namespace Ripes
