#include <QtTest/QTest>

#include "assembler/expreval.h"

using namespace Ripes;
using namespace Ripes::Assembler;

class tst_ExprEval : public QObject {
  Q_OBJECT

private slots:
  void tst_binops();
};

void expect(const ExprEvalRes &res, const ExprEvalVT &expected) {
  if (auto *err = std::get_if<Error>(&res)) {
    QString errstr = "Got error: " + err->toString();
    QFAIL(errstr.toStdString().c_str());
  }
  QCOMPARE(std::get<ExprEvalVT>(res), expected);
}

void tst_ExprEval::tst_binops() {
  expect(evaluate(Location::unknown(), "(0x2*(3+4))+4"), 18);
  expect(evaluate(Location::unknown(), "2+3*7*5"), 107);
  expect(evaluate(Location::unknown(), "1+2*3+4"), 11);

  // Higher-precedence operators bind tighter: * / % over + -, + - over &,
  // and & over |.
  expect(evaluate(Location::unknown(), "2+3*4"), 14);
  expect(evaluate(Location::unknown(), "2*3+4"), 10);
  expect(evaluate(Location::unknown(), "2*3-1"), 5);
  expect(evaluate(Location::unknown(), "20/2-3"), 7);
  expect(evaluate(Location::unknown(), "1+2*3-4/2"), 5);
  expect(evaluate(Location::unknown(), "1+2*3+4*5"), 27);
  expect(evaluate(Location::unknown(), "1+3&2"), 0);
  expect(evaluate(Location::unknown(), "1+1&3"), 2);
  expect(evaluate(Location::unknown(), "6&3|8"), 10);
  expect(evaluate(Location::unknown(), "1|2&3"), 3);

  // Operators of equal precedence are evaluated left to right.
  expect(evaluate(Location::unknown(), "10-2-3"), 5);
  expect(evaluate(Location::unknown(), "3-2+1"), 2);
  expect(evaluate(Location::unknown(), "8/4/2"), 1);
  expect(evaluate(Location::unknown(), "100/10*2"), 20);
  expect(evaluate(Location::unknown(), "10%4*3"), 6);
  expect(evaluate(Location::unknown(), "64/4%5"), 1);

  // Unary minus binds tightest; parentheses override precedence.
  expect(evaluate(Location::unknown(), "-5+3"), -2);
  expect(evaluate(Location::unknown(), "5*-3"), -15);
  expect(evaluate(Location::unknown(), "-2*-3"), 6);
  expect(evaluate(Location::unknown(), "-(2+3)*2"), -10);
  expect(evaluate(Location::unknown(), "(10-2)-3"), 5);
  expect(evaluate(Location::unknown(), "10-(2-3)"), 11);
  expect(evaluate(Location::unknown(), "(2+3)*4"), 20);
  expect(evaluate(Location::unknown(), "2*(3+4)-5%3"), 12);
  expect(evaluate(Location::unknown(), "((7))"), 7);

  // Sign extension (@) binds tighter than + - * / %.
  expect(evaluate(Location::unknown(), "0xF0@8+1"), -15);
  expect(evaluate(Location::unknown(), "2*0xF0@8"), -32);

  SymbolMap symbols;
  Location::unknown(), symbols.abs["B"] = 2;
  expect(evaluate(Location::unknown(), "(B *(3+ 4))+4", &symbols.abs), 18);
  expect(evaluate(Location::unknown(), "B*3+1", &symbols.abs), 7);
  expect(evaluate(Location::unknown(), "B-B-B", &symbols.abs), -2);
}

QTEST_APPLESS_MAIN(tst_ExprEval)
#include "tst_expreval.moc"
