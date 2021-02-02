#include "expreval.h"

#include <iostream>
#include <memory>

#include "assembler_defines.h"
#include "parserutilities.h"

namespace Ripes {
namespace Assembler {

const QRegularExpression s_exprOperatorsRegex = QRegularExpression(R"((\+|\-|\/|\*|\%))");
const QString s_exprOperators QStringLiteral("+-*/%");
const QString s_exprTokens QStringLiteral("()+-*/%");

#define IfExpr(TExpr, boundVar) if (auto* boundVar = std::get_if<TExpr>(expr.get())) {
#define FiExpr }

struct Expr;

struct Printable {
    virtual void print(std::ostream& str) const = 0;
};

struct Literal : Printable {
    Literal(QString _v) : v(_v) {}
    QString v;
    void print(std::ostream& str) const override;
};

struct Add : Printable {
    Add(std::shared_ptr<Expr> _lhs, std::shared_ptr<Expr> _rhs) : lhs(_lhs), rhs(_rhs) {}
    std::shared_ptr<Expr> lhs, rhs;
    void print(std::ostream& str) const override;
};

struct Sub : Printable {
    Sub(std::shared_ptr<Expr> _lhs, std::shared_ptr<Expr> _rhs) : lhs(_lhs), rhs(_rhs) {}
    std::shared_ptr<Expr> lhs, rhs;
    void print(std::ostream& str) const override;
};

struct Mul : Printable {
    Mul(std::shared_ptr<Expr> _lhs, std::shared_ptr<Expr> _rhs) : lhs(_lhs), rhs(_rhs) {}
    std::shared_ptr<Expr> lhs, rhs;
    void print(std::ostream& str) const override;
};

struct Div : Printable {
    Div(std::shared_ptr<Expr> _lhs, std::shared_ptr<Expr> _rhs) : lhs(_lhs), rhs(_rhs) {}
    std::shared_ptr<Expr> lhs, rhs;
    void print(std::ostream& str) const override;
};

struct Mod : Printable {
    Mod(std::shared_ptr<Expr> _lhs, std::shared_ptr<Expr> _rhs) : lhs(_lhs), rhs(_rhs) {}
    std::shared_ptr<Expr> lhs, rhs;
    void print(std::ostream& str) const override;
};

struct And : Printable {
    And(std::shared_ptr<Expr> _lhs, std::shared_ptr<Expr> _rhs) : lhs(_lhs), rhs(_rhs) {}
    std::shared_ptr<Expr> lhs, rhs;
    void print(std::ostream& str) const override;
};

struct Or : Printable {
    Or(std::shared_ptr<Expr> _lhs, std::shared_ptr<Expr> _rhs) : lhs(_lhs), rhs(_rhs) {}
    std::shared_ptr<Expr> lhs, rhs;
    void print(std::ostream& str) const override;
};

struct Expr : std::variant<Literal, Add, Mul, Div, Sub, Mod, And, Or> {
    using variant::variant;

    /**
     * @brief operator <<
     * This also seem quite dumb, same issue as with evaluate (see comment)
     */
    friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Expr>& expr) {
#define tryPrint(type) \
    IfExpr(type, v) {  \
        v->print(os);  \
        return os;     \
    }                  \
    FiExpr;

        tryPrint(Add);
        tryPrint(Div);
        tryPrint(Mul);
        tryPrint(Sub);
        tryPrint(Literal);
        tryPrint(Mod);

        return os;
    };
};

// clang-format off
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
// clang-format on

using ExprRes = std::variant<Error, std::shared_ptr<Expr>>;
ExprRes parseRight(const QString& s, int& pos, int& depth);

template <typename BinOp>
ExprRes rightRec(std::shared_ptr<Expr> lhs, const QString& s, int& pos, int& depth) {
    auto rhs = parseRight(s, pos, depth);
    if (auto* err = std::get_if<Error>(&rhs)) {
        return {*err};
    }
    return {std::make_shared<Expr>(BinOp{lhs, std::get<std::shared_ptr<Expr>>(rhs)})};
}

#define Token(lhs) std::make_shared<Expr>(Literal{lhs})

ExprRes parseLeft(const QString& s, int& pos, int& depth);
ExprRes parseRight(const QString& s, int& pos, int& depth) {
    QString lhs;
    while (pos < s.length()) {
        // clang-format off
        auto& ch = s.at(pos);
        pos++;
        switch (ch.unicode()) {
            case '(': { depth++; return parseLeft(s, pos, depth);}
            case ')': { return depth-- != 0 ? Token(lhs) : ExprRes(Error(-1, "Unmatched parenthesis in expression '" + s + '"'));};
            case '+': { return rightRec<Add>(Token(lhs), s, pos, depth);}
            case '/': { return rightRec<Div>(Token(lhs), s, pos, depth);}
            case '*': { return rightRec<Mul>(Token(lhs), s, pos, depth);}
            case '-': { return rightRec<Sub>(Token(lhs), s, pos, depth);}
            case '%': { return rightRec<Mod>(Token(lhs), s, pos, depth);}
            case '|': { return rightRec<Or>(Token(lhs), s, pos, depth);}
            case '&': { return rightRec<And>(Token(lhs), s, pos, depth);}
            default:  {lhs.append(ch);}
        }
        // clang-format on
    }
    return Token(lhs);
}

ExprRes parseLeft(const QString& s, int& pos, int& depth) {
    auto left = parseRight(s, pos, depth);
    if (auto* err = std::get_if<Error>(&left)) {
        return {*err};
    }

    auto& res = std::get<std::shared_ptr<Expr>>(left);
    if (pos < s.length()) {
        auto& ch = s.at(pos);
        pos++;
        // clang-format off
        switch (ch.unicode()) {
            case '+': { return rightRec<Add>(res, s, pos, depth);}
            case '/': { return rightRec<Div>(res, s, pos, depth);}
            case '|': { return rightRec<Or>(res, s, pos, depth);}
            case '&': { return rightRec<And>(res, s, pos, depth);}
            case '*': { return rightRec<Mul>(res, s, pos, depth);}
            case '-': { return rightRec<Sub>(res, s, pos, depth);}
            case '%': { return rightRec<Mod>(res, s, pos, depth);}
            case ')': { return depth-- != 0 ? res : ExprRes(Error(-1, "Unmatched parenthesis in expression '" + s + '"'));};
            default:  { return ExprRes(Error(-1, "Invalid operator '" + QString(ch) + "' in expression '" + s + "'"));}
        }
        // clang-format on
    } else {
        return left;
    }
}

long evaluate(const std::shared_ptr<Expr>& expr, const SymbolMap* variables) {
    // There is a bug in GCC for variant visitors on incomplete variant types (recursive), So instead we'll macro
    // our way towards something that looks like a pattern match for the variant type.
    IfExpr(Add, v) { return evaluate(v->lhs, variables) + evaluate(v->rhs, variables); }
    FiExpr;
    IfExpr(Div, v) { return evaluate(v->lhs, variables) / evaluate(v->rhs, variables); }
    FiExpr;
    IfExpr(Mul, v) { return evaluate(v->lhs, variables) * evaluate(v->rhs, variables); }
    FiExpr;
    IfExpr(Sub, v) { return evaluate(v->lhs, variables) - evaluate(v->rhs, variables); }
    FiExpr;
    IfExpr(Mod, v) { return evaluate(v->lhs, variables) % evaluate(v->rhs, variables); }
    FiExpr;
    IfExpr(And, v) { return evaluate(v->lhs, variables) & evaluate(v->rhs, variables); }
    FiExpr;
    IfExpr(Or, v) { return evaluate(v->lhs, variables) | evaluate(v->rhs, variables); }
    FiExpr;
    IfExpr(Literal, v) {
        bool canConvert;
        auto value = getImmediate(v->v, canConvert);
        if (!canConvert) {
            if (variables != nullptr && variables->count(v->v) != 0) {
                value = variables->at(v->v);
            } else {
                throw std::runtime_error(QString("Unknown symbol '%1'").arg(v->v).toStdString());
            }
        }
        return value;
    }
    FiExpr;

    Q_UNREACHABLE();
}

std::variant<Error, long> evaluate(const QString& s, const SymbolMap* variables) {
    QString sNoWhitespace = s;
    sNoWhitespace.replace(" ", "");
    int pos = 0;
    int depth = 0;
    auto exprTree = parseLeft(sNoWhitespace, pos, depth);
    if (auto* err = std::get_if<Error>(&exprTree)) {
        return *err;
    }
    const auto exprTreeRes = std::get<std::shared_ptr<Expr>>(exprTree);
    try {
        return {evaluate(exprTreeRes, variables)};
    } catch (const std::runtime_error& e) {
        return {Error(-1, e.what())};
    }
}

bool couldBeExpression(const QString& s) {
    return std::any_of(s_exprTokens.begin(), s_exprTokens.end(), [&s](const auto& ch) { return s.contains(ch); });
}

}  // namespace Assembler
}  // namespace Ripes
