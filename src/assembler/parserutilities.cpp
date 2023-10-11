#include "parserutilities.h"
#include "binutils.h"

#include <memory>

namespace Ripes {
namespace Assembler {

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
