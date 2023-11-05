#include "gnudirectives.h"
#include "assembler.h"

namespace Ripes {
namespace Assembler {

static QString numTokensError(unsigned expected, const TokenizedSrcLine &line) {
  QString err = QString::fromStdString(
      "Invalid number of directive arguments; expected " +
      std::to_string(expected) + " but got " +
      std::to_string(line.tokens.size()) + "[");
  for (auto t : llvm::enumerate(line.tokens)) {
    err += t.value();
    if (static_cast<int64_t>(t.index()) < (line.tokens.size() - 1))
      err += ", ";
  };
  err += "].";
  return err;
}

#define add_directive(container, directive)                                    \
  container.push_back(std::make_shared<Directive>(directive));

Result<int64_t> tryGetImmediate(const AssemblerBase *assembler,
                                const Token &token, Location location) {
  auto res = assembler->evalExpr(location, token);
  if (auto *err = std::get_if<Error>(&res))
    return {*err};
  auto immPtr = dynamic_cast<IntRes *>(res->get());
  if (!immPtr)
    return {Error(location, "Expected immediate value")};
  return immPtr->v;
}
template <size_t size>
std::optional<Error> assembleData(const AssemblerBase *assembler,
                                  const TokenizedSrcLine &line,
                                  QByteArray &byteArray) {
  static_assert(size >= 1, "");
  for (const auto &token : line.tokens) {
    auto valOrErr = tryGetImmediate(assembler, token, line);
    if (failed(valOrErr))
      return {valOrErr.error()};
    int64_t val = valOrErr.value();
    static_assert(sizeof(val) >= size,
                  "Requested data width greater than what is representable");

    if (isUInt<size * 8>(val) || isInt<size * 8>(val)) {
      for (size_t i = 0; i < size; ++i) {
        byteArray.append(val & 0xff);
        val >>= 8;
      }
    } else {
      return {Error(
          line, QString("'%1' does not fit in %2 bytes").arg(val).arg(size))};
    }
  }
  return {};
}

std::optional<Error> assembleFloatLike(const AssemblerBase *assembler,
                                       const TokenizedSrcLine &line,
                                       QByteArray &byteArray, bool isDouble) {
  for (const auto &token : line.tokens) {
    // By default, parse all values as doubles. Then, if !isDouble, we check
    // whether the value fits in a regular float.
  }
  return {};
}

template <size_t size>
Result<QByteArray> dataFunctor(const AssemblerBase *assembler,
                               const DirectiveArg &arg) {
  if (arg.line.tokens.length() < 1) {
    return {Error(arg.line, "Invalid number of arguments (expected >1)")};
  }
  QByteArray bytes;
  auto err = assembleData<size>(assembler, arg.line, bytes);
  if (err) {
    return {err.value()};
  } else {
    return {bytes};
  }
}

static Result<QByteArray> stringFunctor(const AssemblerBase *,
                                        const DirectiveArg &arg) {
  if (arg.line.tokens.length() != 1) {
    return Result<QByteArray>{Error(arg.line, numTokensError(1, arg.line))};
  }
  QString string = arg.line.tokens.at(0);
  string.replace("\\n", "\n");
  string.remove('\"');
  return {string.toUtf8().append('\0')};
}

Directive ascizDirective() { return Directive(".asciz", &stringFunctor); }
Directive byteDirective() { return Directive(".byte", &dataFunctor<1>); }
Directive dwordDirective() { return Directive(".dword", &dataFunctor<8>); }
Directive wordDirective() { return Directive(".word", &dataFunctor<4>); }
Directive halfDirective() { return Directive(".half", &dataFunctor<2>); }
Directive shortDirective() { return Directive(".short", &dataFunctor<2>); }
Directive twoByteDirective() { return Directive(".2byte", &dataFunctor<2>); }
Directive fourByteDirective() { return Directive(".4byte", &dataFunctor<4>); }
Directive longDirective() { return Directive(".long", &dataFunctor<4>); }
Directive stringDirective() { return Directive(".string", &stringFunctor); }
Directive floatDirective() { return Directive(".float", &dataFunctor<4>); }
Directive doubleDirective() { return Directive(".double", &dataFunctor<8>); }

/**
 * @brief dummyDirective
 * Generates a directive handler for @p name, which returns nothing. To be used
 * for compatability reasons.
 */
Directive dummyDirective(const QString &name) {
  return Directive(
      name,
      [](const AssemblerBase *, const DirectiveArg &) -> Result<QByteArray> {
        return {QByteArray()};
      });
}

Directive::DirectiveHandler genSegmentChangeFunctor(const QString &segment) {
  return [segment](const AssemblerBase *assembler, const DirectiveArg &arg) {
    if (arg.line.tokens.length() != 0) {
      return Result<QByteArray>{Error(arg.line, numTokensError(0, arg.line))};
    }
    auto err = assembler->setCurrentSegment(arg.line, segment);
    if (err)
      return Result<QByteArray>{err.value()};
    return Result<QByteArray>(QByteArray());
  };
}

Directive textDirective() {
  return Directive(".text", genSegmentChangeFunctor(".text"));
}

Directive bssDirective() {
  return Directive(".bss", genSegmentChangeFunctor(".bss"));
}

Directive dataDirective() {
  return Directive(".data", genSegmentChangeFunctor(".data"));
}

Directive zeroDirective() {
  auto zeroFunctor = [](const AssemblerBase *assembler,
                        const DirectiveArg &arg) -> Result<QByteArray> {
    if (arg.line.tokens.length() != 1) {
      return Result<QByteArray>{Error(arg.line, numTokensError(1, arg.line))};
    }
    auto valOrErr = tryGetImmediate(assembler, arg.line.tokens.at(0), arg.line);
    if (failed(valOrErr))
      return {valOrErr.error()};
    int64_t value = valOrErr.value();

    QByteArray bytes;
    bytes.fill(0x0, value);
    return {bytes};
  };
  return Directive(".zero", zeroFunctor);
}

Directive equDirective() {
  auto equFunctor = [](const AssemblerBase *assembler,
                       const DirectiveArg &arg) -> Result<QByteArray> {
    if (arg.line.tokens.length() != 2) {
      return Result<QByteArray>{Error(arg.line, numTokensError(2, arg.line))};
    }
    auto valOrErr = tryGetImmediate(assembler, arg.line.tokens.at(1), arg.line);
    if (failed(valOrErr))
      return {valOrErr.error()};
    int64_t value = valOrErr.value();

    auto err = assembler->m_symbolMap.addSymbol(arg.line, arg.line.tokens.at(0),
                                                value);
    if (err) {
      return err.value();
    }

    return Result<QByteArray>(QByteArray());
  };
  return Directive(".equ", equFunctor,
                     true /* Constants should be made available during ie. pseudo instruction expansion */);
}

Directive alignDirective() {
  auto alignFunctor = [](const AssemblerBase *assembler,
                         const DirectiveArg &arg) -> Result<QByteArray> {
    if (arg.line.tokens.length() == 0 || arg.line.tokens.length() > 3) {
      return {Error(
          arg.line,
          "Invalid number of arguments (expected at least 1, at most 3)")};
    }
    int boundary, fill, max;
    fill = max = 0;
    bool hasMax = false;

    auto boundaryOrErr =
        tryGetImmediate(assembler, arg.line.tokens.at(0), arg.line);
    if (failed(boundaryOrErr))
      return {boundaryOrErr.error()};
    boundary = boundaryOrErr.value();

    if (arg.line.tokens.size() > 1) {
      auto fillOrErr =
          tryGetImmediate(assembler, arg.line.tokens.at(1), arg.line);
      if (failed(fillOrErr))
        return {fillOrErr.error()};
      fill = fillOrErr.value();
    }
    if (arg.line.tokens.size() > 2) {
      auto maxOrErr =
          tryGetImmediate(assembler, arg.line.tokens.at(2), arg.line);
      if (failed(maxOrErr))
        return {maxOrErr.error()};
      max = maxOrErr.value();
      hasMax = true;
    }

    if (boundary < 0 || fill < 0 || (hasMax && max < 0)) {
      return {Error(arg.line, ".align arguments must be positive")};
    }
    if (fill > UINT8_MAX) {
      return {Error(arg.line, ".align fill value must be in range [0;255]")};
    }
    if (boundary == 0) {
      return {QByteArray()};
    }
    int byteOffset =
        (arg.section->address + arg.section->data.size()) % boundary;
    int bytesToSkip = byteOffset != 0 ? boundary - byteOffset : 0;
    if (max > 0 && bytesToSkip > max) {
      return {QByteArray()};
    }

    return {QByteArray(1, static_cast<char>(fill)).repeated(bytesToSkip)};
  };
  return Directive(".align", alignFunctor);
}

DirectiveVec gnuDirectives() {
  DirectiveVec directives;

  add_directive(directives, stringDirective());
  add_directive(directives, ascizDirective());
  add_directive(directives, zeroDirective());
  add_directive(directives, byteDirective());
  add_directive(directives, dwordDirective());
  add_directive(directives, wordDirective());
  add_directive(directives, halfDirective());
  add_directive(directives, shortDirective());
  add_directive(directives, twoByteDirective());
  add_directive(directives, fourByteDirective());
  add_directive(directives, longDirective());
  add_directive(directives, equDirective());
  add_directive(directives, alignDirective());

  add_directive(directives, floatDirective());
  add_directive(directives, doubleDirective());

  add_directive(directives, dataDirective());
  add_directive(directives, textDirective());
  add_directive(directives, bssDirective());

  add_directive(directives, dummyDirective(".global"));
  add_directive(directives, dummyDirective(".globl"));

  return directives;
}

} // namespace Assembler
} // namespace Ripes
