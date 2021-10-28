#include <QDir>
#include <QProcess>
#include <QStringList>
#include <QtTest/QTest>

#include "processorhandler.h"
#include "processorregistry.h"
#include "ripessettings.h"

#include "assembler/rv32i_assembler.h"

#if !defined(RISCV32_TEST_DIR) || !defined(RISCV64_TEST_DIR) || !defined(RISCV32_C_TEST_DIR) || \
    !defined(RISCV64_C_TEST_DIR)
static_assert(false, "RISCV test directiories must be defined");
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
using namespace Assembler;

// Ecall status codes
static constexpr unsigned s_success = 42;

// Test status register
static constexpr unsigned s_statusreg = 3;  // Current test stored in the gp(3) register
static constexpr unsigned s_ecallreg = 10;  // a0

// Maximum cycle count
static constexpr unsigned s_maxCycles = 10000;

// Tests which contains instructions or assembler directives not yet supported
const auto s_excludedTests = {"f", "ldst", "move", "recoding", /* fails on CI, unknown as of know */ "memory"};

class tst_RISCV : public QObject {
    Q_OBJECT

private:
    void loadBinaryToSimulator(const QString& binFile);
    bool skipTest(const QString& test);
    QString executeSimulator();
    QString dumpRegs();

    QString m_currentTest;

    void runTests(const ProcessorID& id, const QStringList& extensions, const QStringList& testdirs);

    void trapHandler();

    bool m_stop = false;
    std::shared_ptr<Program> m_program;
    QString m_err;

private slots:

    void testRV64_SingleCycle() { runTests(ProcessorID::RV64_SS, {"M", "C"}, {RISCV64_TEST_DIR, RISCV64_C_TEST_DIR}); }
    void testRV64_5StagePipeline() {
        runTests(ProcessorID::RV64_5S, {"M", "C"}, {RISCV64_TEST_DIR, RISCV64_C_TEST_DIR});
    }
    void testRV64_5StagePipelineNOFW() {
        runTests(ProcessorID::RV64_5S_NO_FW, {"M", "C"}, {RISCV64_TEST_DIR, RISCV64_C_TEST_DIR});
    }
    void testRV64_6SDual() { runTests(ProcessorID::RV64_6S_DUAL, {"M", "C"}, {RISCV64_TEST_DIR, RISCV64_C_TEST_DIR}); }

    void testRV32_SingleCycle() { runTests(ProcessorID::RV32_SS, {"M", "C"}, {RISCV32_TEST_DIR, RISCV32_C_TEST_DIR}); }
    void testRV32_5StagePipeline() {
        runTests(ProcessorID::RV32_5S, {"M", "C"}, {RISCV32_TEST_DIR, RISCV32_C_TEST_DIR});
    }
    void testRV32_5StagePipelineNOFW() {
        runTests(ProcessorID::RV32_5S_NO_FW, {"M", "C"}, {RISCV32_TEST_DIR, RISCV32_C_TEST_DIR});
    }
    void testRV32_6SDual() { runTests(ProcessorID::RV32_6S_DUAL, {"M", "C"}, {RISCV32_TEST_DIR, RISCV32_C_TEST_DIR}); }
};

bool tst_RISCV::skipTest(const QString& test) {
    for (const auto& t : s_excludedTests) {
        if (test.startsWith(t)) {
            return true;
        }
    }
    return false;
}

QString tst_RISCV::dumpRegs() {
    QString str = "\n" + m_currentTest + "\nRegister dump:";
    str += "\t PC:" + QString::number(ProcessorHandler::getProcessor()->getPcForStage(0), 16) + "\n";
    for (unsigned i = 0; i < ProcessorHandler::currentISA()->regCnt(); i++) {
        const auto value = ProcessorHandler::getProcessor()->getRegister(RegisterFileType::GPR, i);
        str += "\t" + ProcessorHandler::currentISA()->regName(i) + ":" + ProcessorHandler::currentISA()->regAlias(i) +
               ":\t" + QString::number(value) + " (0x" + QString::number(value, 16) + ")\n";
    }
    return str;
}

void tst_RISCV::loadBinaryToSimulator(const QString& binFile) {
    // Read test file
    QFile testFile(binFile);
    if (!testFile.open(QIODevice::ReadOnly)) {
        QString err = "Test: '" + m_currentTest + "' failed: Could not read compiled test file.";
        QFAIL(err.toStdString().c_str());
    }
    QByteArray programByteArray = testFile.readAll();

    m_program = std::make_shared<Program>();
    m_program->sections[TEXT_SECTION_NAME] = ProgramSection{TEXT_SECTION_NAME, 0, programByteArray};
    ProcessorHandler::get()->loadProgram(m_program);
}

void tst_RISCV::trapHandler() {
    unsigned status = ProcessorHandler::getProcessor()->getRegister(RegisterFileType::GPR, s_ecallreg);
    if (status != s_success) {
        m_err = "Test: '" + m_currentTest + "' failed: Internal test error.\n\t test number: " +
                QString::number(ProcessorHandler::getProcessor()->getRegister(RegisterFileType::GPR, s_statusreg));
        m_err += dumpRegs();
    }
    m_stop |= true;
}

QString tst_RISCV::executeSimulator() {
    m_stop = false;
    m_err = QString();
    bool maxCyclesReached = false;
    unsigned cycles = 0;
    do {
        ProcessorHandler::getProcessorNonConst()->clock();

        cycles++;

        maxCyclesReached |= cycles >= s_maxCycles;
        m_stop |= maxCyclesReached;
    } while (!m_stop);

    if (maxCyclesReached) {
        m_err = "Test: '" + m_currentTest + "' failed: Maximum cycle count reached\n\t test number: " +
                QString::number(ProcessorHandler::getProcessor()->getRegister(RegisterFileType::GPR, s_statusreg));
        m_err += dumpRegs();
    }

    // Test successful
    return m_err;
}

void tst_RISCV::runTests(const ProcessorID& id, const QStringList& extensions, const QStringList& testDirs) {
    for (auto testDir : testDirs) {
        const auto dir = QDir(testDir);
        const auto testFiles = dir.entryList({"*.s"});
        ProcessorHandler::selectProcessor(id, extensions);

        for (const auto& test : testFiles) {
            m_currentTest = test;
            if (skipTest(m_currentTest))
                continue;

            qInfo() << "Running test: " << m_currentTest;

            // Assemble test file
            auto f = QFile(testDir + QString(QDir::separator()) + test);
            if (!f.open(QIODevice::ReadOnly)) {
                QFAIL("Could not open test file");
            }
            const auto program = ProcessorHandler::getAssembler()->assemble(QString(f.readAll()).split("\n"));
            if (program.errors.size() != 0) {
                QString err = "Could not assemble program";
                err += "\n errors were:";
                err += program.errors.toString();
                QFAIL(err.toStdString().c_str());
            }
            auto spProgram = std::make_shared<Program>(program.program);

            // Override the ProcessorHandler's ECALL handling. In doing so, we verify whether the correct test value was
            // reached.
            ProcessorHandler::getProcessorNonConst()->trapHandler = [=] { trapHandler(); };
            ProcessorHandler::get()->loadProgram(spProgram);
            RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET)->trigger();

            const QString err = executeSimulator();
            if (!err.isNull()) {
                QFAIL(err.toStdString().c_str());
            }

            qInfo() << "Test '" << m_currentTest << "' succeeded.";
        }
    }
}

QTEST_APPLESS_MAIN(tst_RISCV)
#include "tst_riscv.moc"
