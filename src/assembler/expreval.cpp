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

#define IfExpr(TExpr, boundVar)                                                \
  if (auto *boundVar = std::get_if<TExpr>(expr.get())) {
#define FiExpr }

struct Expr;

struct Printable {
  virtual ~Printable(){};
  virtual void print(std::ostream &str) const = 0;
};

struct Nothing : Printable {
  Nothing() {}
  void print(std::ostream &str) const override;
};

struct Literal : Printable {
  explicit Literal(const QString &_v) : v(_v) {}
  QString v;
  void print(std::ostream &str) const override;
};

struct Add : Printable {
  Add(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Sub : Printable {
  Sub(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Mul : Printable {
  Mul(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Div : Printable {
  Div(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Mod : Printable {
  Mod(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct And : Printable {
  And(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Or : Printable {
  Or(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct SignExtend : Printable {
  SignExtend(const std::shared_ptr<Expr> &_lhs,
             const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
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
ExprRes parseRight(const Location &loc, const QString &s, int &pos, int &depth);

template <typename BinOp>
ExprRes rightRec(const Location &loc, const std::shared_ptr<Expr> &lhs,
                 const QString &s, int &pos, int &depth) {
  auto rhs = parseRight(loc, s, pos, depth);
  if (auto *err = std::get_if<Error>(&rhs)) {
    return {*err};
  }
  return {
      std::make_shared<Expr>(BinOp{lhs, std::get<std::shared_ptr<Expr>>(rhs)})};
}

#define Token(lhs) std::make_shared<Expr>(Literal{lhs})

ExprRes parseLeft(const Location &loc, const QString &s, int &pos, int &depth);
ExprRes parseRight(const Location &loc, const QString &s, int &pos,
                   int &depth) {
  QString lhs;
  while (pos < s.length()) {
    // clang-format off
        auto& ch = s.at(pos);
        pos++;
        switch (ch.unicode()) {
            case '(': { depth++; return parseLeft(loc, s, pos, depth);}
            case ')': { return depth-- != 0 ? Token(lhs) : ExprRes(Error(loc, "Unmatched parenthesis in expression '" + s + '"'));};
            case '+': { return rightRec<Add>(loc, Token(lhs), s, pos, depth);}
            case '/': { return rightRec<Div>(loc, Token(lhs), s, pos, depth);}
            case '*': { return rightRec<Mul>(loc, Token(lhs), s, pos, depth);}
            case '-': {
                auto lhsToken = lhs.isEmpty() ? std::make_shared<Expr>(Nothing()) : Token(lhs); // Allow unary '-'
                return rightRec<Sub>(loc, lhsToken, s, pos, depth);}
            case '%': { return rightRec<Mod>(loc, Token(lhs), s, pos, depth);}
            case '|': { return rightRec<Or>(loc, Token(lhs), s, pos, depth);}
            case '&': { return rightRec<And>(loc, Token(lhs), s, pos, depth);}
            case '@': { return rightRec<SignExtend>(loc, Token(lhs), s, pos, depth);}
            default:  {lhs.append(ch);}
        }
    // clang-format on
  }
  return Token(lhs);
}

ExprRes parseLeft(const Location &loc, const QString &s, int &pos, int &depth) {
  auto left = parseRight(loc, s, pos, depth);
  if (auto *err = std::get_if<Error>(&left)) {
    return {*err};
  }

  const auto &res = std::get<std::shared_ptr<Expr>>(left);
  if (pos < s.length()) {
    auto &ch = s.at(pos);
    pos++;
    // clang-format off
        switch (ch.unicode()) {
            case '+': { return rightRec<Add>(loc, res, s, pos, depth);}
            case '/': { return rightRec<Div>(loc, res, s, pos, depth);}
            case '|': { return rightRec<Or> (loc, res, s, pos, depth);}
            case '&': { return rightRec<And>(loc, res, s, pos, depth);}
            case '*': { return rightRec<Mul>(loc, res, s, pos, depth);}
            case '-': { return rightRec<Sub>(loc, res, s, pos, depth);}
            case '%': { return rightRec<Mod>(loc, res, s, pos, depth);}
            case '@': { return rightRec<SignExtend>(loc, res, s, pos, depth);}
            case ')': { return depth-- != 0 ? res : ExprRes(Error(loc, "Unmatched parenthesis in expression '" + s + '"'));};
            default:  { return ExprRes(Error(-1, "Invalid operator '" + QString(ch) + "' in expression '" + s + "'"));}
        }
    // clang-format on
  } else {
    return left;
  }
}

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
    return evaluate(v->lhs, variables) / evaluate(v->rhs, variables);
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
    return evaluate(v->lhs, variables) % evaluate(v->rhs, variables);
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
  int pos = 0;
  int depth = 0;
  auto exprTree = parseLeft(loc, sNoWhitespace, pos, depth);
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
