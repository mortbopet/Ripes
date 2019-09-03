#include <QtTest/QtTest>

#include "mainwindow.h"
#include "pipeline.h"

#include <map>

class TestRipes : public QObject {
    Q_OBJECT
private slots:
    void runTests();

private:
    void executeTest(const QString& testcase);
    void executeTest(const QString& testcase, const std::map<int, uint32_t>& verificationMap);
};

std::map<int, uint32_t> parseVerificationString(const QString& testcase) {
    QString verificationString;
    QFile file(testcase);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        verificationString = file.readAll();
        file.close();
    } else {
        Q_ASSERT(false);
    }

    std::map<int, uint32_t> vparms;
    verificationString.truncate(verificationString.indexOf('\n'));

    // Test case must start with a verification string
    Q_ASSERT(verificationString.startsWith('#'));
    verificationString = verificationString.remove(0, 1).trimmed();

    auto parms = verificationString.split(' ');
    for (const auto& parm : parms) {
        const auto& kv = parm.split(':');
        Q_ASSERT(kv.size() == 2);
        int key = kv[0].toInt();
        uint32_t value = kv[1].toUInt(nullptr, 16);
        vparms[key] = value;
    }
    return vparms;
}

void TestRipes::executeTest(const QString& testcase) {
    const auto& verificationMap = parseVerificationString(testcase);
    executeTest(testcase, verificationMap);
}

void TestRipes::executeTest(const QString& testcase, const std::map<int, uint32_t>& verificationMap) {
    MainWindow w;
    w.loadAssemblyFile(testcase);

    auto pipeline = Pipeline::getPipeline();

    // After loading an assembly file, the gui will await for no further changes by the user, and then start assembling.
    // Wait for this sequence to finish by polling whether the pipeline has loaded a program.
    while (pipeline->getTextSize() == 0) {
        QTest::qWait(100);
    }

    w.run();

    const auto& regs = *pipeline->getRegPtr();
    for (const auto& v : verificationMap) {
        if (regs[v.first] != v.second) {
            QString msg = QString("Failed in test: %1\n").arg(testcase);
            msg += QString("    Reg: %1     Expected: %2    Actual: %3")
                       .arg(QString::number(v.first), QString::number(v.second), QString::number(regs[v.first]));
            QFAIL(msg.toStdString().c_str());
        }
    }
}

void TestRipes::runTests() {
    // Avoid some warnings related to missing icons in the linked Ripes library
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(examples);

    // Test the built-in examples
    executeTest(":/examples/assembly/Complex multiplication",
                {{5, 19}, {7, -7}, {10, 1}, {11, 19}, {12, 5}, {13, 4}, {28, 15}});
    executeTest(":/examples/assembly/Console printing", {
                                                            {5, 54},
                                                            {6, 53},
                                                            {10, 10},
                                                            {11, 268435472},
                                                            {12, 2},
                                                        });
    executeTest(":/examples/assembly/Factorial function", {{5, 7}, {6, 5040}, {10, 10}, {11, 5040}});
    executeTest(":/examples/assembly/Large program",
                {{6, 328}, {10, 10}, {11, 100}, {13, 3200}, {14, 100}, {15, 4950}});

    // Execute all tests in the tests.qrc file
    QDirIterator it(":/tests/", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        executeTest(it.next());
    }
}

QTEST_MAIN(TestRipes)
#include "test.moc"
