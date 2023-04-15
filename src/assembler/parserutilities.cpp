#include "parserutilities.h"
#include "binutils.h"

#include <memory>

namespace Ripes {
namespace Assembler {

int64_t getImmediate(const QString &string, bool &canConvert,
                     ImmConvInfo *convInfo) {
  QString upperString = string.toUpper();
  QString trimmed;
  canConvert = false;
  int64_t immediate = upperString.toLongLong(&canConvert, 10);
  int64_t sign = 1;
  if (!canConvert) {
    // Could not convert directly to integer - try hex or bin. Here, extra care
    // is taken to account for a potential sign, and include this is the range
    // validation
    if (upperString.size() > 0 &&
        (upperString.at(0) == '-' || upperString.at(0) == '+')) {
      sign = upperString.at(0) == '-' ? -1 : 1;
      upperString.remove(0, 1);
    }
    if (upperString.startsWith(QLatin1String("0X"))) {
      trimmed = upperString.remove("0X");
      if (convInfo) {
        convInfo->isUnsigned = true;
        convInfo->is32bit = trimmed.size() <= 8;
        convInfo->radix = Radix::Hex;
      }
      immediate = trimmed.toULongLong(&canConvert, 16);
    } else if (upperString.startsWith(QLatin1String("0B"))) {
      trimmed = upperString.remove("0B");
      if (convInfo) {
        convInfo->isUnsigned = true;
        convInfo->is32bit = trimmed.size() <= 32;
        convInfo->radix = Radix::Binary;
      }
      immediate = trimmed.toULongLong(&canConvert, 2);
    } else {
      canConvert = false;
    }
  } else {
    if (convInfo) {
      convInfo->radix = Radix::Signed;
    }
  }

  return sign * immediate;
}

int64_t getImmediateSext32(const QString &string, bool &success,
                           ImmConvInfo *convInfo) {
  std::unique_ptr<ImmConvInfo> innerConvInfo;
  if (!convInfo) {
    innerConvInfo = std::make_unique<ImmConvInfo>();
    convInfo = innerConvInfo.get();
  }

  int64_t value = getImmediate(string, success, convInfo);

  // This seems a tad too specific for RISC-V, but the official RISC-V tests
  // expects the immediate of i.e., "andi x14, x1, 0xffffff0f" to be accepted as
  // a signed immediate, even in 64-bit.
  if (success && (static_cast<uint32_t>(value >> 32) == 0) &&
      convInfo->is32bit) {
    value = static_cast<int32_t>(value);
  }
  return value;
}

/**
 * @brief joinParentheses takes a number of tokens and merges together tokens
 * contained within top-level parentheses. For example: [lw, x10, (B, +,
 * (3*2))(x10)] => [lw, x10, B + 3*2), x10]
 */
bool matchedParens(std::vector<QChar> &parensStack, QChar end) {
  if (parensStack.size() == 0) {
    return false;
  }
  const QChar toMatch = parensStack.at(parensStack.size() - 1);
  parensStack.pop_back();
  return (toMatch == '[' && end == ']') || (toMatch == '(' && end == ')');
}

Result<LineTokens> joinParentheses(const Location &loc,
                                   const QStringList &tokens) {
  LineTokens outtokens;
  std::vector<QChar> parensStack;

  QString tokenBuffer;
  auto commitBuffer = [&]() {
    if (!tokenBuffer.isEmpty()) {
      outtokens << Token(tokenBuffer);
      tokenBuffer.clear();
    }
  };

  for (const auto &token : tokens) {
    if (token.startsWith("\"") && token.endsWith("\"")) {
      // String literal; ignore parentheses inside
      outtokens << Token(token);
      continue;
    }
    for (const auto &ch : token) {
      switch (ch.unicode()) {
      case '(':
      case '[':
        if (!parensStack.empty()) {
          tokenBuffer.append(ch);
        } else {
          commitBuffer();
        }
        parensStack.push_back(ch);
        break;
      case ']':
      case ')': {
        if (matchedParens(parensStack, ch)) {
          if (parensStack.empty()) {
            commitBuffer();
          } else {
            tokenBuffer.append(ch);
          }
        } else {
          return {Error(loc, "Unmatched parenthesis")};
        }
        break;
      }
      default:
        tokenBuffer.append(ch);
        break;
      }
    }
    if (parensStack.empty()) {
      commitBuffer();
    }
  }

  if (parensStack.empty()) {
    return outtokens;
  } else {
    return {Error(loc, "Unmatched parenthesis")};
  }
}

Result<QStringList> tokenizeQuotes(const Location &location,
                                   const QString &line) {
  QStringList tokens;
  bool inQuotes = false;
  bool escape = false;
  QString substr;
  auto pushSubstr = [&] {
    tokens.push_back(substr);
    substr.clear();
  };
  for (auto &ch : line) {
    if (inQuotes) {
      if (!escape) {
        if (inQuotes && ch == '"') {
          inQuotes = false;
          substr += ch;
          pushSubstr();
          continue;
        }
        if (ch == '\\')
          escape = true;
      } else
        escape = false;
      substr.push_back(ch);
    } else {
      if (ch == ' ' || ch == ',' || ch == '\t') {
        if (!substr.isEmpty())
          pushSubstr();
      } else
        substr.push_back(ch);
      if (ch == '\"')
        inQuotes = true;
    }
  }

  if (inQuotes)
    return {Error(location, "Missing terminating '\"' character.")};

  tokens.push_back(substr);
  tokens.removeAll(QLatin1String(""));
  return {tokens};
}

} // namespace Assembler
} // namespace Ripes
