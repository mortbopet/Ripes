#include "expreval.h"

#include <iostream>
#include <memory>

#include "assembler_defines.h"
#include "binutils.h"
#include "parserutilities.h"

namespace Ripes {
namespace Assembler {

const QRegularExpression s_exprOperatorsRegex =
    QRegularExpression(R"((\+|\-|\/|\*|\%|\@))");
const QString s_exprOperators QStringLiteral("+-*/%@");
const QString s_exprTokens QStringLiteral("()+-*/%@");

// TExpr is a type and boundVar a declarator; neither can be parenthesized.
// NOLINTBEGIN(bugprone-macro-parentheses)
#define IfExpr(TExpr, boundVar)                                                \
  if (auto *boundVar = std::get_if<TExpr>(expr.get())) {
// NOLINTEND(bugprone-macro-parentheses)
#define FiExpr }

struct Expr;

struct Printable {
  virtual ~Printable() = default;
  virtual void print(std::ostream &str) const = 0;
};

struct Nothing : Printable {
  Nothing() = default;
  void print(std::ostream &os) const override;
};

struct Literal : Printable {
  explicit Literal(const QString &_v) : v(_v) {}
  QString v;
  void print(std::ostream &os) const override;
};

struct Add : Printable {
  Add(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &os) const override;
};

struct Sub : Printable {
  Sub(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &os) const override;
};

struct Mul : Printable {
  Mul(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &os) const override;
};

struct Div : Printable {
  Div(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &os) const override;
};

struct Mod : Printable {
  Mod(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &os) const override;
};

struct And : Printable {
  And(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &os) const override;
};

struct Or : Printable {
  Or(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &os) const override;
};

struct SignExtend : Printable {
  SignExtend(const std::shared_ptr<Expr> &_lhs,
             const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &os) const override;
};

struct Expr : std::variant<Literal, Add, Mul, Div, Sub, Mod, And, Or, Nothing,
                           SignExtend> {
  using variant::variant;

  /**
   * @brief operator <<
   * This also seem quite dumb, same issue as with evaluate (see comment)
   */
  friend std::ostream &operator<<(std::ostream &os,
                                  const std::shared_ptr<Expr> &expr) {
#define tryPrint(type)                                                         \
  IfExpr(type, v) {                                                            \
    v->print(os);                                                              \
    return os;                                                                 \
  }                                                                            \
  FiExpr;

    tryPrint(Add);
    tryPrint(Div);
    tryPrint(Mul);
    tryPrint(Sub);
    tryPrint(Literal);
    tryPrint(Mod);
    tryPrint(Nothing);
    tryPrint(SignExtend);

    return os;
  };
};

// clang-format off
void Nothing::print(std::ostream&) const {}
void Add::print(std::ostream& os) const {
    os << "(" << lhs << " + " << rhs << ")";
}
void Sub::print(std::ostream& os) const {
    os << "(" << lhs << " - " << rhs << ")";
}
void Mul::print(std::ostream& os) const {
    os << "(" << lhs << " * " << rhs << ")";
}
void Div::print(std::ostream& os) const {
    os << "(" << lhs << " / " << rhs << ")";
}
void Mod::print(std::ostream& os) const {
    os << "(" << lhs << " % " << rhs << ")";
}
void Literal::print(std::ostream& os) const {
    os << v.toStdString();
}
void And::print(std::ostream& os) const {
    os << "(" << lhs << " & " << rhs << ")";
}
void Or::print(std::ostream& os) const {
    os << "(" << lhs << " | " << rhs << ")";
}

void SignExtend::print(std::ostream& os) const {
    os << "(" << lhs << " @ " << rhs << ")";
}

// clang-format on

using ExprRes = Result<std::shared_ptr<Expr>>;

namespace {

/// Characters which end a literal/symbol token.
const QString s_exprStopChars QStringLiteral("()+-*/%@|&");

/// Builds the AST node matching a binary operator character.
std::shared_ptr<Expr> makeBinOp(QChar op, const std::shared_ptr<Expr> &lhs,
                                const std::shared_ptr<Expr> &rhs) {
  switch (op.unicode()) {
  case '|':
    return std::make_shared<Expr>(Or{lhs, rhs});
  case '&':
    return std::make_shared<Expr>(And{lhs, rhs});
  case '+':
    return std::make_shared<Expr>(Add{lhs, rhs});
  case '-':
    return std::make_shared<Expr>(Sub{lhs, rhs});
  case '*':
    return std::make_shared<Expr>(Mul{lhs, rhs});
  case '/':
    return std::make_shared<Expr>(Div{lhs, rhs});
  case '%':
    return std::make_shared<Expr>(Mod{lhs, rhs});
  case '@':
    return std::make_shared<Expr>(SignExtend{lhs, rhs});
  }
  Q_UNREACHABLE();
}

/**
 * Precedence-climbing (recursive descent) expression parser.
 * Lowest to highest precedence, all binary operators left-associative:
 *   | -> & -> + - -> * / % -> @ -> unary - -> ( ) / literal
 */
struct ExprParser {
  const Location &loc;
  const QString &s;
  int pos = 0;

  bool atEnd() const { return pos >= s.length(); }
  QChar peek() const { return atEnd() ? QChar() : s.at(pos); }

  ExprRes fail(const QString &msg) const {
    return ExprRes(Error(loc, msg + " in expression '" + s + "'"));
  }

  /// Parses: next (op next)*  for any op in 'ops', folding to the left.
  ExprRes parseBinary(const QString &ops, ExprRes (ExprParser::*next)()) {
    auto lhsRes = (this->*next)();
    if (auto *err = std::get_if<Error>(&lhsRes))
      return ExprRes(*err);
    auto lhs = std::get<std::shared_ptr<Expr>>(lhsRes);

    while (!atEnd() && ops.contains(peek())) {
      const QChar op = s.at(pos++);
      auto rhsRes = (this->*next)();
      if (auto *err = std::get_if<Error>(&rhsRes))
        return ExprRes(*err);
      lhs = makeBinOp(op, lhs, std::get<std::shared_ptr<Expr>>(rhsRes));
    }
    return ExprRes(lhs);
  }

  ExprRes parseOr() {
    return parseBinary(QStringLiteral("|"), &ExprParser::parseAnd);
  }
  ExprRes parseAnd() {
    return parseBinary(QStringLiteral("&"), &ExprParser::parseAdditive);
  }
  ExprRes parseAdditive() {
    return parseBinary(QStringLiteral("+-"), &ExprParser::parseMultiplicative);
  }
  ExprRes parseMultiplicative() {
    return parseBinary(QStringLiteral("*/%"), &ExprParser::parseSignExtend);
  }
  ExprRes parseSignExtend() {
    return parseBinary(QStringLiteral("@"), &ExprParser::parseUnary);
  }

  /// Unary minus: '-x' is represented as Sub(Nothing, x) == 0 - x.
  ExprRes parseUnary() {
    if (peek() == '-') {
      pos++;
      auto operandRes = parseUnary();
      if (auto *err = std::get_if<Error>(&operandRes))
        return ExprRes(*err);
      return ExprRes(std::make_shared<Expr>(
          Sub{std::make_shared<Expr>(Nothing()),
              std::get<std::shared_ptr<Expr>>(operandRes)}));
    }
    return parsePrimary();
  }

  /// A parenthesized sub-expression, or a literal/symbol.
  ExprRes parsePrimary() {
    if (atEnd())
      return fail("Missing operand");

    if (peek() == '(') {
      pos++;
      auto inner = parseOr();
      if (std::holds_alternative<Error>(inner))
        return inner;
      if (peek() != ')')
        return fail("Unmatched parenthesis");
      pos++;
      return inner;
    }

    const int start = pos;
    while (!atEnd() && !s_exprStopChars.contains(peek()))
      pos++;
    if (pos == start)
      return fail("Missing operand before '" + QString(peek()) + "'");
    return ExprRes(std::make_shared<Expr>(Literal{s.mid(start, pos - start)}));
  }

  /// Entry point: parse the whole string, and reject any leftover input.
  ExprRes parse() {
    auto res = parseOr();
    if (std::holds_alternative<Error>(res))
      return res;
    if (!atEnd()) {
      if (peek() == ')')
        return fail("Unmatched parenthesis");
      return fail("Unexpected '" + QString(peek()) + "'");
    }
    return res;
  }
};

} // namespace

VIntS evaluate(const std::shared_ptr<Expr> &expr,
               const AbsoluteSymbolMap *variables) {
  // There is a bug in GCC for variant visitors on incomplete variant types
  // (recursive), So instead we'll macro our way towards something that looks
  // like a pattern match for the variant type.
  IfExpr(Add, v) {
    return evaluate(v->lhs, variables) + evaluate(v->rhs, variables);
  }
  FiExpr;
  IfExpr(Div, v) {
    auto rhs_value = evaluate(v->rhs, variables);
    if (rhs_value == 0) {
      throw std::runtime_error(
          "Division by zero error in expression evaluation.");
    }

    return evaluate(v->lhs, variables) / rhs_value;
  }
  FiExpr;
  IfExpr(Mul, v) {
    return evaluate(v->lhs, variables) * evaluate(v->rhs, variables);
  }
  FiExpr;
  IfExpr(Sub, v) {
    return evaluate(v->lhs, variables) - evaluate(v->rhs, variables);
  }
  FiExpr;
  IfExpr(Mod, v) {
    auto rhs_value = evaluate(v->rhs, variables);
    if (rhs_value == 0) {
      throw std::runtime_error(
          "Modulo by zero error in expression evaluation.");
    }

    return evaluate(v->lhs, variables) % rhs_value;
  }
  FiExpr;
  IfExpr(And, v) {
    return evaluate(v->lhs, variables) & evaluate(v->rhs, variables);
  }
  FiExpr;
  IfExpr(Or, v) {
    return evaluate(v->lhs, variables) | evaluate(v->rhs, variables);
  }
  FiExpr;
  IfExpr(SignExtend, v) {
    return vsrtl::signextend(evaluate(v->lhs, variables),
                             evaluate(v->rhs, variables));
  }
  FiExpr;
  IfExpr(Nothing, v) {
    Q_UNUSED(v);
    return 0;
  }
  FiExpr;
  IfExpr(Literal, v) {
    bool ok = false;
    auto value = getImmediate(v->v, ok);
    if (!ok) {
      if (variables != nullptr) {
        auto it = variables->find(v->v);
        if (it != variables->end()) {
          value = it->second;
          ok = true;
        }
      }
    }

    if (!ok) {
      throw std::runtime_error(
          QString("Unknown symbol '%1'").arg(v->v).toStdString());
    }

    return value;
  }
  FiExpr;

  Q_UNREACHABLE();
}

ExprEvalRes evaluate(const Location &loc, const QString &s,
                     const AbsoluteSymbolMap *variables) {
  QString sNoWhitespace = s;
  sNoWhitespace.replace(" ", "");
  ExprParser parser{loc, sNoWhitespace};
  auto exprTree = parser.parse();
  if (auto *err = std::get_if<Error>(&exprTree)) {
    return *err;
  }
  const auto exprTreeRes = std::get<std::shared_ptr<Expr>>(exprTree);
  try {
    return {evaluate(exprTreeRes, variables)};
  } catch (const std::runtime_error &e) {
    return {Error(loc, e.what())};
  }
}

bool couldBeExpression(const QString &s) {
  return std::any_of(s_exprTokens.begin(), s_exprTokens.end(),
                     [&s](const auto &ch) { return s.contains(ch); });
}

} // namespace Assembler
} // namespace Ripes
