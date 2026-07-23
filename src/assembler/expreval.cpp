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
const QRegularExpression s_exprFloatRegex = QRegularExpression(
    R"(^[+-]?(?!0[xb])((\d+\.\d*)|(\.\d+)|(\d+))(e[+-]?\d+)?)",
    QRegularExpression::CaseInsensitiveOption);
const QString s_exprOperators QStringLiteral("+-*/%@");
const QString s_exprTokens QStringLiteral("()+-*/%@");

struct Expr {
  virtual ~Expr(){};
  virtual QString print() const = 0;
  virtual ExprEvalVT evaluate(const AbsoluteSymbolMap *variables) const = 0;

  friend std::ostream &operator<<(std::ostream &os,
                                  const std::shared_ptr<Expr> &expr) {
    os << expr->print().toStdString();
    return os;
  }
};

struct Nothing : public Expr {
  Nothing() {}
  QString print() const override { return ""; }
  ExprEvalVT evaluate(const AbsoluteSymbolMap *variables) const override {
    Q_UNUSED(variables);
    return {ExprEvalIntType{0}};
  }
};

struct Literal : public Expr {
  explicit Literal(const QString &_v) : v(_v) {}
  QString v;
  QString print() const override { return v; }
  ExprEvalVT evaluate(const AbsoluteSymbolMap *variables) const override {
    ImmConvInfo convInfo;
    bool ok = false;
    auto value = getImmediate(v, ok, &convInfo);

    if (!ok) {
      if (variables != nullptr) {
        auto it = variables->find(v);
        if (it != variables->end()) {
          value = it->second;
          ok = true;
        }
      }
    }

    if (!ok) {
      throw std::runtime_error(
          QString("Unknown symbol '%1'").arg(v).toStdString());
    }

    // if the literal was parsed as a float, return as such
    if (convInfo.radix == Radix::Float) {
      return {ExprEvalFloatType{.word = static_cast<uint32_t>(value)}};
    }

    // else return as integer
    return {ExprEvalIntType{value}};
  }
};

struct ExprBinOp : public Expr {
  std::shared_ptr<Expr> lhs, rhs;

  explicit ExprBinOp(const std::shared_ptr<Expr> &_lhs,
                     const std::shared_ptr<Expr> &_rhs)
      : lhs(_lhs), rhs(_rhs) {}

  virtual ~ExprBinOp(){};
  QString print() const override = 0;
  virtual ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                                  const ExprEvalIntType &rhs) const = 0;
  virtual ExprEvalFloatType evalFloat(const ExprEvalFloatType &lhs,
                                      const ExprEvalFloatType &rhs) const = 0;

  ExprEvalVT evaluate(const AbsoluteSymbolMap *variables) const override {
    auto lhsVal = lhs->evaluate(variables);
    auto rhsVal = rhs->evaluate(variables);

    // Integer operation
    if (std::holds_alternative<ExprEvalIntType>(lhsVal) &&
        std::holds_alternative<ExprEvalIntType>(rhsVal)) {
      return {evalInt(std::get<ExprEvalIntType>(lhsVal),
                      std::get<ExprEvalIntType>(rhsVal))};
    }

    // Float operation
    ExprEvalFloatType lhsF, rhsF;
    if (std::holds_alternative<ExprEvalIntType>(lhsVal)) {
      lhsF = ExprEvalFloatType::from<ExprEvalIntType>(
          std::get<ExprEvalIntType>(lhsVal));
    } else {
      lhsF = std::get<ExprEvalFloatType>(lhsVal);
    }

    if (std::holds_alternative<ExprEvalIntType>(rhsVal)) {
      rhsF = ExprEvalFloatType::from<ExprEvalIntType>(
          std::get<ExprEvalIntType>(rhsVal));
    } else {
      rhsF = std::get<ExprEvalFloatType>(rhsVal);
    }

    return {evalFloat(lhsF, rhsF)};
  }
};
struct ExprBinIntegerOp : public ExprBinOp {
  explicit ExprBinIntegerOp(const std::shared_ptr<Expr> &_lhs,
                            const std::shared_ptr<Expr> &_rhs)
      : ExprBinOp(_lhs, _rhs) {}
  virtual ~ExprBinIntegerOp(){};
  QString print() const override = 0;
  virtual ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                                  const ExprEvalIntType &rhs) const = 0;
  ExprEvalFloatType evalFloat(const ExprEvalFloatType &lhs,
                              const ExprEvalFloatType &rhs) const override {
    Q_UNUSED(lhs);
    Q_UNUSED(rhs);
    throw std::runtime_error("operation not defined for float types");
  }
};

struct Add : public ExprBinOp {
  Add(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : ExprBinOp(_lhs, _rhs) {}

  QString print() const override {
    return "(" + lhs->print() + " + " + rhs->print() + ")";
  }
  ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                          const ExprEvalIntType &rhs) const override {
    return lhs + rhs;
  }
  ExprEvalFloatType evalFloat(const ExprEvalFloatType &lhs,
                              const ExprEvalFloatType &rhs) const override {
    return lhs + rhs;
  }
};

struct Sub : public ExprBinOp {
  Sub(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : ExprBinOp(_lhs, _rhs) {}

  QString print() const override {
    return "(" + lhs->print() + " - " + rhs->print() + ")";
  }
  ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                          const ExprEvalIntType &rhs) const override {
    return lhs - rhs;
  }
  ExprEvalFloatType evalFloat(const ExprEvalFloatType &lhs,
                              const ExprEvalFloatType &rhs) const override {
    return lhs - rhs;
  }
};

struct Mul : public ExprBinOp {
  Mul(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : ExprBinOp(_lhs, _rhs) {}

  QString print() const override {
    return "(" + lhs->print() + " * " + rhs->print() + ")";
  }
  ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                          const ExprEvalIntType &rhs) const override {
    return lhs * rhs;
  }
  ExprEvalFloatType evalFloat(const ExprEvalFloatType &lhs,
                              const ExprEvalFloatType &rhs) const override {
    return lhs * rhs;
  }
};

struct Div : public ExprBinOp {
  Div(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : ExprBinOp(_lhs, _rhs) {}

  QString print() const override {
    return "(" + lhs->print() + " / " + rhs->print() + ")";
  }
  ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                          const ExprEvalIntType &rhs) const override {
    if (rhs == 0) {
      throw std::runtime_error(
          "Division by zero error in expression evaluation.");
    }
    return lhs / rhs;
  }
  ExprEvalFloatType evalFloat(const ExprEvalFloatType &lhs,
                              const ExprEvalFloatType &rhs) const override {
    if (rhs.word == 0) {
      throw std::runtime_error(
          "Division by zero error in expression evaluation.");
    }
    return lhs / rhs;
  }
};

struct Mod : public ExprBinOp {
  Mod(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : ExprBinOp(_lhs, _rhs) {}

  QString print() const override {
    return "(" + lhs->print() + " % " + rhs->print() + ")";
  }
  ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                          const ExprEvalIntType &rhs) const override {
    if (rhs == 0) {
      throw std::runtime_error(
          "Division by zero error in expression evaluation.");
    }
    return lhs % rhs;
  }
  ExprEvalFloatType evalFloat(const ExprEvalFloatType &lhs,
                              const ExprEvalFloatType &rhs) const override {
    if (rhs.word == 0) {
      throw std::runtime_error(
          "Division by zero error in expression evaluation.");
    }
    return lhs % rhs;
  }
};

struct And : public ExprBinIntegerOp {
  And(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : ExprBinIntegerOp(_lhs, _rhs) {}

  QString print() const override {
    return "(" + lhs->print() + " & " + rhs->print() + ")";
  }
  ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                          const ExprEvalIntType &rhs) const override {
    return lhs & rhs;
  }
};

struct Or : public ExprBinIntegerOp {
  Or(const std::shared_ptr<Expr> &_lhs, const std::shared_ptr<Expr> &_rhs)
      : ExprBinIntegerOp(_lhs, _rhs) {}

  QString print() const override {
    return "(" + lhs->print() + " | " + rhs->print() + ")";
  }
  ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                          const ExprEvalIntType &rhs) const override {
    return lhs | rhs;
  }
};

struct SignExtend : public ExprBinIntegerOp {
  SignExtend(const std::shared_ptr<Expr> &_lhs,
             const std::shared_ptr<Expr> &_rhs)
      : ExprBinIntegerOp(_lhs, _rhs) {}

  QString print() const override {
    return "(" + lhs->print() + " @ " + rhs->print() + ")";
  }
  ExprEvalIntType evalInt(const ExprEvalIntType &lhs,
                          const ExprEvalIntType &rhs) const override {
    return vsrtl::signextend(lhs, rhs);
  }
};

using ExprRes = Result<std::shared_ptr<Expr>>;
ExprRes parseRight(const Location &loc, const QString &s, int &pos, int &depth);
ExprRes parseLeft(const Location &loc, const QString &s, int &pos, int &depth);

template <typename BinOp>
ExprRes rightRec(const Location &loc, const std::shared_ptr<Expr> &lhs,
                 const QString &s, int &pos, int &depth) {
  auto rhs = parseRight(loc, s, pos, depth);
  if (auto *err = std::get_if<Error>(&rhs)) {
    return {*err};
  }
  return {std::make_shared<BinOp>(lhs, std::get<std::shared_ptr<Expr>>(rhs))};
}

#define Token(lhs) std::make_shared<Literal>(lhs)
ExprRes parseRight(const Location &loc, const QString &s, int &pos,
                   int &depth) {

  QString lhs;

  // try parsing as float literal
  // since float literals can contain +/- which would get parsed as binary
  // operators we need to parse out float literals beforehand
  QRegularExpressionMatch match = s_exprFloatRegex.match(s, pos);
  if (match.hasMatch() && match.capturedStart() == pos) {
    lhs = match.captured(0);
    pos += lhs.length();
  }

  while (pos < s.length()) {
    // clang-format off
        auto& ch = s.at(pos);
        pos++;
        switch (ch.unicode()) {
            case '(': { depth++; return parseLeft(loc, s, pos, depth);}
            case ')': { return depth-- != 0 ? ExprRes(Token(lhs)) : ExprRes(Error(loc, "Unmatched parenthesis in expression '" + s + '"'));};
            case '+': { return rightRec<Add>(loc, Token(lhs), s, pos, depth);}
            case '/': { return rightRec<Div>(loc, Token(lhs), s, pos, depth);}
            case '*': { return rightRec<Mul>(loc, Token(lhs), s, pos, depth);}
            case '-': {
              if (lhs.isEmpty()) {
                return rightRec<Sub>(loc, std::make_shared<Nothing>(), s, pos, depth);
              } else {
                return rightRec<Sub>(loc, Token(lhs), s, pos, depth);
              }
            }
            case '%': { return rightRec<Mod>(loc, Token(lhs), s, pos, depth);}
            case '|': { return rightRec<Or>(loc, Token(lhs), s, pos, depth);}
            case '&': { return rightRec<And>(loc, Token(lhs), s, pos, depth);}
            case '@': { return rightRec<SignExtend>(loc, Token(lhs), s, pos, depth);}
            default:  {lhs.append(ch);}
        }
    // clang-format on
  }
  return ExprRes(Token(lhs));
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
    return {exprTreeRes->evaluate(variables)};
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
