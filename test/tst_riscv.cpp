#include <QDir>
#include <QProcess>
#include <QStringList>
#include <QtTest/QTest>

#include "../src/processors/ss/riscv_ss.h"

#ifndef VSRTL_RISCV_TEST_DIR
static_assert(false, "VSRTL_RISCV_TEST_DIR must be defined");
#endif

/** RISC-V test suite
 *
 * For now, the following assumptions are made:
 * - When compiling, it is assumed that the entry point address is 0x0
 * - No .data segment is contained within the resulting .ELF file
 * As such, we directly copy the .text segment into the simulator memory and execute the test.
 */

using namespace Ripes;
using namespace vsrtl::core;

// Compilation tools & directories
const QString s_testdir = VSRTL_RISCV_TEST_DIR;
const QString s_outdir = s_testdir + QDir::separator() + "build";
const QString s_assembler = "riscv64-linux-gnu-as";
const QString s_objcopy = "riscv64-linux-gnu-objcopy";
const QString s_linkerScript = "rvtest.ld";

// Ecall status codes
static constexpr unsigned s_success = 42;
static constexpr unsigned s_fail = 0;

// Test status register
static constexpr unsigned s_statusreg = 3;  // Current test stored in the gp(3) register
static constexpr unsigned s_ecallreg = 10;  // a0

// Maximum cycle count
static constexpr unsigned s_maxCycles = 10000;

// Tests which contains instructions or assembler directives not yet supported
const auto s_excludedTests = {"f", "ldst", "move", "recoding"};

QString compileTestFile(const QString& testfile) {
    QProcess exec;

    const QString outElf = s_outdir + QDir::separator() + testfile + ".out";
    const QString outBin = s_outdir + QDir::separator() + testfile + ".bin";

    if (!QDir(s_outdir).exists()) {
        QDir().mkdir(s_outdir);
    }

    // Build
    bool error = exec.execute(s_assembler, {"-march=rv32im", s_testdir + QDir::separator() + testfile, "-o", outElf});

    // Extract .text segment
    error = exec.execute(s_objcopy, {"-O", "binary", "--only-section=.text", outElf, outBin});

    return error ? QString() : outBin;
}

class tst_RISCV : public QObject {
    Q_OBJECT

private:
    void loadBinaryToSimulator(const QString& binFile);
    bool skipTest(const QString& test);
    QString executeSimulator();

    QString m_currentTest;
    std::unique_ptr<SingleCycleRISCV> m_design;

private slots:
    void runTests();

    void cleanupTestCase();
};

void tst_RISCV::cleanupTestCase() {
    auto buildDir = QDir(s_outdir);
    buildDir.removeRecursively();
}

bool tst_RISCV::skipTest(const QString& test) {
    for (const auto& t : s_excludedTests) {
        if (test.startsWith(t)) {
            return true;
        }
    }
    return false;
}

void tst_RISCV::loadBinaryToSimulator(const QString& binFile) {
    // Read test file
    QFile testFile(binFile);
    if (!testFile.open(QIODevice::ReadOnly)) {
        QString err = "Test: '" + m_currentTest + "' failed: Could not read compiled test file.";
        QFAIL(err.toStdString().c_str());
    }
    QByteArray programByteArray = testFile.readAll();
    std::vector<unsigned char> program(programByteArray.begin(), programByteArray.end());

    // Load program into simulator
    m_design->m_memory->addInitializationMemory(0x0, program.data(), program.size());
}

QString tst_RISCV::executeSimulator() {
    bool stop = false;
    bool maxCyclesReached = false;
    unsigned cycles = 0;
    do {
        m_design->clock();
        cycles++;

        maxCyclesReached |= cycles >= s_maxCycles;
        stop |= maxCyclesReached;
        if (m_design->decode->opcode.uValue() == RVInstr::ECALL) {
            unsigned status = m_design->registerFile->getRegister(s_ecallreg);
            if (status == s_success) {
                stop |= true;
            } else if (status == s_fail) {
                return "Test: '" + m_currentTest + "' failed: Internal test error.\n\t test number: " +
                       QString::number(m_design->registerFile->getRegister(s_statusreg));
            }
        }
    } while (!stop);

    if (maxCyclesReached) {
        return "Test: '" + m_currentTest + "' failed: Maximum cycle count reached\n\t test number: " +
               QString::number(m_design->registerFile->getRegister(s_statusreg));
    }

    // Test successful
    return QString();
}

void tst_RISCV::runTests() {
    const auto dir = QDir(s_testdir);
    const auto testFiles = dir.entryList({"*.s"});

    for (const auto& test : testFiles) {
        m_currentTest = test;

        if (skipTest(m_currentTest))
            continue;

        // Compile test file
        const auto binFile = compileTestFile(test);
        if (binFile.isNull()) {
            QString err = "Test: '" + test + "' failed: Could not compile test file.";
            QFAIL(err.toStdString().c_str());
        }

        m_design = std::make_unique<SingleCycleRISCV>();
        loadBinaryToSimulator(binFile);
        m_design->verifyAndInitialize();

        QString err = executeSimulator();
        if (!err.isNull()) {
            QFAIL(err.toStdString().c_str());
        }
    }
}

QTEST_APPLESS_MAIN(tst_RISCV)
#include "tst_riscv.moc"
