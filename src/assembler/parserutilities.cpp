#include "parserutilities.h"
#include "binutils.h"

#include <memory>

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
bool matchedParens(std::vector<QChar> &parensStack, QChar end) {
  if (parensStack.empty()) {
    return false;
  }
  const QChar toMatch = parensStack.at(parensStack.size() - 1);
  parensStack.pop_back();
  return (toMatch == '[' && end == ']') || (toMatch == '(' && end == ')');
}

Result<LineTokens> joinParentheses(const Location &loc,
                                   const QStringList &tokens) {
  // Operators which tie a parenthesized group to the expression around it.
  static const QString s_exprOperators = QStringLiteral("+-*/%&|@");

  LineTokens outtokens;
  std::vector<QChar> parensStack;
  // Whether the current top-level group is part of a surrounding expression,
  // e.g. the (3+4) in 2*(3+4), rather than an operand of its own.
  bool groupInExpression = false;

  QString tokenBuffer;
  auto commitBuffer = [&](bool parenthesized = false) {
    if (!tokenBuffer.isEmpty()) {
      Token token(tokenBuffer);
      token.setParenthesized(parenthesized);
      outtokens << token;
      tokenBuffer.clear();
    }
  };

  for (const auto &token : tokens) {
    if (token.startsWith("\"") && token.endsWith("\"")) {
      // String literal; ignore parentheses inside
      outtokens << Token(token);
      continue;
    }
    for (qsizetype i = 0; i < token.size(); ++i) {
      const QChar ch = token.at(i);
      switch (ch.unicode()) {
      case '(':
      case '[':
        if (!parensStack.empty()) {
          tokenBuffer.append(ch);
        } else if (!tokenBuffer.isEmpty() &&
                   s_exprOperators.contains(tokenBuffer.back())) {
          // An operator precedes the group, e.g. 2*(3+4): keep the group within
          // the expression.
          groupInExpression = true;
          tokenBuffer.append(ch);
        } else {
          // The group is an operand of its own, e.g. the base register in
          // 0(t0) or the argument of %pcrel_hi(sym).
          groupInExpression = false;
          commitBuffer();
        }
        parensStack.push_back(ch);
        break;
      case ']':
      case ')': {
        if (!matchedParens(parensStack, ch)) {
          return {Error(loc, "Unmatched parenthesis")};
        }
        if (!parensStack.empty() || groupInExpression) {
          tokenBuffer.append(ch);
        } else if (i + 1 < token.size() &&
                   s_exprOperators.contains(token.at(i + 1))) {
          // An operator follows the group, e.g. (2+3)*4: the group starts an
          // expression.
          tokenBuffer.prepend(ch == ')' ? QChar('(') : QChar('['));
          tokenBuffer.append(ch);
        } else {
          commitBuffer(/*parenthesized=*/true);
        }
        if (parensStack.empty()) {
          groupInExpression = false;
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
  }
  return {Error(loc, "Unmatched parenthesis")};
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
        if (ch == '"') {
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
