#pragma once

#include "assembler_defines.h"
#include "assemblererror.h"
#include "symbolmap.h"
#include <QRegularExpression>
#include <variant>

namespace Ripes {
namespace Assembler {

extern const QRegularExpression s_exprOperatorsRegex;
extern const QString s_exprOperators;
extern const QString s_exprTokens;
using ExprEvalVT =
    std::variant<double, int64_t>; // Expression evaluation value type

// The result of an expression evaluation - we use inheritance here to allow for
// TypeSwitch usage.
struct ExprValue {
  virtual ~ExprValue() = default;
};

struct DoubleRes : public ExprValue {
  DoubleRes(double v) : v(v) {}
  double v;
};

struct IntRes : public ExprValue {
  IntRes(int64_t v) : v(v) {}
  int64_t v;
};

using ExprEvalRes = Result<std::shared_ptr<ExprValue>>;

/**
 * @brief evaluate
 * Very simple expression parser for evaluating a right-associative binary
 * (2-operand) mathematical expressions. For now, no operator precedence is
 * implemented - to ensure precedence, parentheses must be implemented. The
 * functionality is mainly intended to be used by the assembler to expand
 * complex pseudoinstructions and as such not by the user.
 * Successfully parsed expressions can evaluate as either a double or an
 * int64_t.
 */
ExprEvalRes evaluate(const Location &, const QString &,
                     const AbsoluteSymbolMap *variables = nullptr);

/**
 * @brief couldBeExpression
 * @returns true if we have probably cause that the string is an expression and
 * not 'just' a single variable.
 */
bool couldBeExpression(const QString &s);
} // namespace Assembler
} // namespace Ripes
