#include <QtTest/QTest>

#include "assembler/expreval.h"

using namespace Ripes::Assembler;

class tst_ExprEval : public QObject {
    Q_OBJECT

private slots:
    void tst_binops();
};

void expect(const ExprEvalRes& res, const ExprEvalVT& expected) {
    if (auto* err = std::get_if<Error>(&res)) {
        QString errstr = "Got error: " + err->second;
        QFAIL(errstr.toStdString().c_str());
    }
    QCOMPARE(std::get<ExprEvalVT>(res), expected);
}

void tst_ExprEval::tst_binops() {
    expect(evaluate("(0x2*(3+4))+4"), 18);
    expect(evaluate("2+3*7*5"), 107);
    SymbolMap symbols;
    symbols["B"] = 2;
    expect(evaluate("(B *(3+ 4))+4", &symbols), 18);
}

QTEST_APPLESS_MAIN(tst_ExprEval)
#include "tst_expreval.moc"
