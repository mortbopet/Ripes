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

    // After loading an assembly file, the gui will await for no further changes by the user, and then start assembling.
    // Sleep, allowing the event loop to be processed
    QTest::qWait(1000);

    auto pipeline = Pipeline::getPipeline();

    pipeline->run();

    const auto& regs = *pipeline->getRegPtr();
    for (const auto& v : verificationMap) {
        QVERIFY(regs[v.first] == v.second);
    }
}

void TestRipes::runTests() {
    // Avoid some warnings related to missing icons in the linked Ripes library
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(examples);

    executeTest(":/tests/mulh.s");
    executeTest(":/tests/mulhu.s");
    executeTest(":/tests/mulhsu.s");
}

QTEST_MAIN(TestRipes)
#include "test.moc"
