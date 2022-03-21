#include <QtTest/QTest>

#include "assembler/expreval.h"

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
  SymbolMap symbols;
  Location::unknown(), symbols.abs["B"] = 2;
  expect(evaluate(Location::unknown(), "(B *(3+ 4))+4", &symbols.abs), 18);
}

QTEST_APPLESS_MAIN(tst_ExprEval)
#include "tst_expreval.moc"
