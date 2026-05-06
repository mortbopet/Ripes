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
 * contained within top-level parentheses. For example: [lw, x10, (B, +,
 * (3*2))(x10)] => [lw, x10, B + 3*2), x10]
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
