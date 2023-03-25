#pragma once

#include <QStringList>
#include <variant>

#include "assembler_defines.h"
#include "assemblererror.h"
#include "radix.h"

namespace Ripes {
namespace Assembler {

struct ImmConvInfo {
  bool isUnsigned = false;
  bool is32bit = false;
  Radix radix;
};

int64_t getImmediate(const QString &string, bool &canConvert,
                     ImmConvInfo *convInfo = nullptr);
int64_t getImmediateSext32(const QString &string, bool &canConvert,
                           ImmConvInfo *convInfo = nullptr);

/**
 * @brief joinParentheses takes a number of tokens and merges together tokens
 * contained within top-level parentheses. For example: [lw, x10, (B, +,
 * (3*2))(x10)] => [lw, x10, B + 3*2), x10]
 */
Result<LineTokens> joinParentheses(const Location &location,
                                   const QStringList &tokens);

/// Quote-aware string tokenization.
Result<QStringList> tokenizeQuotes(const Location &location,
                                   const QString &line);
} // namespace Assembler
} // namespace Ripes
