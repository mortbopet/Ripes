#pragma once

#include <QStringList>
#include <variant>

#include "assembler_defines.h"
#include "assemblererror.h"

namespace Ripes {
namespace Assembler {

struct ImmConvInfo {
    bool isUnsigned = false;
    bool is32bit = false;
};

int64_t getImmediate(const QString& string, bool& canConvert, ImmConvInfo* convInfo = nullptr);
int64_t getImmediateSext32(const QString& string, bool& canConvert);

/**
 * @brief joinParentheses takes a number of tokens and merges together tokens contained within top-level parentheses.
 * For example:
 * [lw, x10, (B, +, (3*2))(x10)] => [lw, x10, B + 3*2), x10]
 */
std::variant<Error, LineTokens> joinParentheses(const QStringList& tokens);

}  // namespace Assembler
}  // namespace Ripes
