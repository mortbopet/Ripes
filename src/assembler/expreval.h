#pragma once

#include "assembler_defines.h"
#include "isa/symbolmap.h"
#include <QRegularExpression>
#include <variant>

namespace Ripes {
namespace Assembler {

extern const QRegularExpression s_exprOperatorsRegex;
extern const QString s_exprOperators;
extern const QString s_exprTokens;
using ExprEvalVT = int64_t; // Expression evaluation value type
using ExprEvalRes = Result<ExprEvalVT>;

/**
 * @brief evaluate
 * Expression parser for evaluating binary (2-operand) and unary '-'
 * mathematical expressions. Operators follow C-like precedence (lowest to
 * highest): '|', '&', '+ -', '* / %', '@', unary '-'. All binary operators
 * are left-associative; parentheses may be used to override precedence.
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
