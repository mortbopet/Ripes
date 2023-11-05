#include "expreval.h"

#include <iostream>
#include <memory>

#include "assembler_defines.h"
#include "binutils.h"
#include "parserutilities.h"
#include "typeswitch.h"

namespace Ripes {
namespace Assembler {

const QRegularExpression s_exprOperatorsRegex =
    QRegularExpression(R"((\+|\-|\/|\*|\%|\@))");
const QString s_exprOperators QStringLiteral("+-*/%@");
const QString s_exprTokens QStringLiteral("()+-*/%@");

#define IfExpr(TExpr, boundVar)                                                \
  if (auto *boundVar = std::get_if<TExpr>(expr.get())) {
#define FiExpr }

struct Printable {
  virtual ~Printable(){};
  virtual void print(std::ostream &str) const = 0;

  friend std::ostream &operator<<(std::ostream &os,
                                  const std::shared_ptr<Printable> &expr) {
    expr->print(os);
    return os;
  }
};

struct Expr : public Printable {
  virtual ~Expr(){};
  void print(std::ostream &) const override{};
};

struct Nothing : public Expr {
  Nothing() {}
  void print(std::ostream &str) const override;
};

struct Literal : public Expr {
  explicit Literal(const QString &_v) : v(_v) {}
  QString v;
  void print(std::ostream &str) const override;
};

struct Add : public Expr {
  Add(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Sub : public Expr {
  Sub(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Mul : public Expr {
  Mul(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Div : public Expr {
  Div(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Mod : public Expr {
  Mod(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct And : public Expr {
  And(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct Or : public Expr {
  Or(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
};

struct SignExtend : public Expr {
  SignExtend(const std::shared_ptr<Expr> &_lhs,
             const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}
  std::shared_ptr<Expr> lhs, rhs;
  void print(std::ostream &str) const override;
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
  return {std::make_shared<BinOp>(lhs, rhs.value())};
}

#define Token(lhs) std::shared_ptr<Expr>(std::make_shared<Literal>(lhs))

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
                auto lhsToken = lhs.isEmpty() ? std::make_shared<Nothing>() : Token(lhs); // Allow unary '-'
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

  const auto &res = left.value();
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

// A template for generating arithmetic expression evaluators that can
// dispatch (and error check) for both integer or double contexts.
template <typename TExpr, typename IntFunc, typename DoubleFunc>
std::function<ExprEvalRes(const TExpr &, const AbsoluteSymbolMap *)>
genEvaluateArithExprF(IntFunc intFunc, DoubleFunc doubleFunc,
                      bool disableDouble = false) {
  return [&](const TExpr &v,
             const AbsoluteSymbolMap *variables) -> ExprEvalRes {
    ExprEvalRes lhsRes = evaluate(v.lhs, variables);
    ExprEvalRes rhsRes = evaluate(v.rhs, variables);
    // If either result errored, return the error.
    if (failed(lhsRes))
      return lhsRes.error();
    if (failed(rhsRes))
      return rhsRes.error();

    // Check if they are of the same implementing type - for now, we
    // restrict expression evaluation on the same types...
    if (typeid(*lhsRes) != typeid(*rhsRes)) {
      return ExprEvalRes(Error(-1, "Cannot compute on different types"));
    }

    auto resPtr =
        Ripes::TypeSwitch<ExprValue, ExprEvalRes>(lhsRes->get())
            .template Case<IntRes>([&](const auto &lhs) {
              return ExprEvalRes{std::make_shared<IntRes>(intFunc(
                  lhs.v, static_cast<IntRes *>(rhsRes.value().get())->v))};
            })
            .template Case<DoubleRes>([&](const auto &lhs) {
              if (disableDouble) {
                return ExprEvalRes(Error(
                    -1, "Unsupported expression for floating point values"));
              }

              return ExprEvalRes{std::make_shared<DoubleRes>(doubleFunc(
                  lhs.v, static_cast<DoubleRes *>(rhsRes.value().get())->v))};
            });
    return resPtr;
  };
}

template <typename TExpr, typename IntFunc>
std::function<ExprEvalRes(const TExpr &, const AbsoluteSymbolMap *)>
genIntegeronlyEvaluateArithExprF(IntFunc intFunc) {
  auto dummyDoubleF = [](double, double) {
    assert(false);
    return 0.0;
  };

  return genEvaluateArithExprF<TExpr>(intFunc, dummyDoubleF, true);
}

ExprEvalRes evaluate(const std::shared_ptr<Expr> &expr,
                     const AbsoluteSymbolMap *variables) {
  return Ripes::TypeSwitch<Expr, ExprEvalRes>(expr.get())
      .Case<Add>([&](const auto &v) {
        return genEvaluateArithExprF<Add>(std::plus<int64_t>(),
                                          std::plus<double>())(v, variables);
      })
      .Case<Div>([&](const auto &v) {
        return genEvaluateArithExprF<Div>(std::divides<int64_t>(),
                                          std::divides<double>())(v, variables);
      })
      .Case<Mul>([&](const auto &v) {
        return genEvaluateArithExprF<Mul>(std::multiplies<int64_t>(),
                                          std::multiplies<double>())(v,
                                                                     variables);
      })
      .Case<Sub>([&](const auto &v) {
        return genEvaluateArithExprF<Sub>(std::minus<int64_t>(),
                                          std::minus<double>())(v, variables);
      })
      .Case<Mod>([&](const auto &v) {
        return genEvaluateArithExprF<Mod>(std::modulus<int64_t>(),
                                          [](double lhs, double rhs) {
                                            return std::fmod(lhs, rhs);
                                          })(v, variables);
      })
      .Case<And>([&](const auto &v) {
        return genIntegeronlyEvaluateArithExprF<And>(std::bit_and<int64_t>())(
            v, variables);
      })
      .Case<Or>([&](const auto &v) {
        return genIntegeronlyEvaluateArithExprF<Or>(std::bit_or<int64_t>())(
            v, variables);
      })
      .Case<SignExtend>([&](const auto &v) {
        return genIntegeronlyEvaluateArithExprF<SignExtend>(
            [](int64_t lhs, int64_t rhs) {
              return vsrtl::signextend(lhs, rhs);
            })(v, variables);
      })
      .Case<Nothing>([&](const auto &) {
        return ExprEvalRes{std::make_shared<IntRes>(0)};
      })
      .Case<Literal>([&](const auto &v) -> ExprEvalRes {
        // Try to decode as an immediate
        bool ok = false;
        int64_t ivalue = getImmediate(v.v, ok);
        if (ok)
          return {std::make_shared<IntRes>(ivalue)};

        // Try to decode as a double
        double dvalue = getDouble(v.v, ok);
        if (ok)
          return {std::make_shared<DoubleRes>(dvalue)};

        if (!ok) {
          if (variables != nullptr) {
            auto it = variables->find(v.v);
            if (it != variables->end()) {
              return {std::make_shared<IntRes>(it->second)};
            }
          }
        }

        return ExprEvalRes(
            Error(Location::unknown(), "Unknown symbol '" + v.v + "'"));
      })
      .Default([](const auto &v) {
        std::cout << "Unhandled expression type: " << typeid(v).name() << v
                  << std::endl;
        return ExprEvalRes({});
      });
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
  const auto exprTreeRes = exprTree.value();
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
