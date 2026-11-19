#pragma once

#include <QStringList>
#include <variant>

#include "assembler_defines.h"
#include "isa/isa_defines.h"
#include "radix.h"

namespace Ripes {
namespace Assembler {

/**
 * @brief joinParentheses takes a number of tokens and merges together tokens
 * contained within top-level parentheses. A parenthesized group becomes an
 * operand of its own, e.g. the base of an offset(base) memory operand or the
 * argument of a relocation: [lw, x10, (B, +, (3*2))(x10)] =>
 * [lw, x10, B+(3*2), x10]. A group which is directly preceded or followed by an
 * expression operator is instead kept within the surrounding expression:
 * [li, x10, 2*(3+4)] => [li, x10, 2*(3+4)].
 */
Result<LineTokens> joinParentheses(const Location &location,
                                   const QStringList &tokens);

/**
 * @brief Quote-aware string tokenization.
 * separates tokens by either ' ', ',' or '\t' characters
 * and returns a list of those separated strings
 */
Result<QStringList> tokenizeQuotes(const Location &location,
                                   const QString &line);
} // namespace Assembler
} // namespace Ripes
