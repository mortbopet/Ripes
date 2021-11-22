#include <QDir>
#include <QProcess>
#include <QResource>
#include <QStringList>
#include <QtTest/QTest>

#include <optional>

#include "processorhandler.h"
#include "processorregistry.h"

#include "edittab.h"
#include "isa/rvisainfo_common.h"
#include "programloader.h"
#include "ripessettings.h"

using namespace Ripes;
using namespace Assembler;

// This test ensures that the 'reverse' feature works across register and memory writes.

class tst_reverse : public QObject {
    Q_OBJECT

private slots:
    void run_test(const ProcessorID& id, const QStringList& program, unsigned rounds, unsigned frontStep,
                  unsigned backStep, bool toFinish);
    void tst_reverse_regs();
    void tst_reverse_mem();
};

using Registers = std::map<int, VInt>;
static Registers dumpRegs() {
    Registers regs;
    for (unsigned i = 0; i < ProcessorHandler::get()->currentISA()->regCnt(); i++) {
        regs[i] = ProcessorHandler::get()->getProcessor()->getRegister(RegisterFileType::GPR, i);
    }
    return regs;
}

void tst_reverse::run_test(const ProcessorID& id, const QStringList& program, unsigned rounds, unsigned front,
                           unsigned back, bool toFinish) {
    ProcessorHandler::get()->selectProcessor(id, {});
    RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET)->trigger();
    ProcessorHandler::get()->getProcessorNonConst()->trapHandler = [=] {};

    auto loader = new ProgramLoader();
    loader->loadTest(program.join("\n"));
    auto proc = ProcessorHandler::get()->getProcessorNonConst();
    // Step back and forth
    for (unsigned r = 0; r < rounds; ++r) {
        for (unsigned s = 0; s < front; ++s) {
            proc->clock();
        }
        auto r1 = dumpRegs();
        for (unsigned s = 0; s < back; ++s) {
            proc->reverseProcessor();
        }
        auto r2 = dumpRegs();
    }

    if (toFinish) {
        // Step until finished
        while (!proc->finished() && proc->getCycleCount() < 1000)
            proc->clock();

        if (!proc->finished())
            QFAIL("Execution never finished");
    }

    return;
}

void tst_reverse::tst_reverse_regs() {
    for (auto processor : {ProcessorID::RV32_SS, ProcessorID::RV32_5S}) {
        QStringList program = QStringList() << ".text"
                                            << "li x10 0"
                                            << "addi x10 x10 1"
                                            << "addi x10 x10 1"
                                            << "addi x10 x10 1"
                                            << "addi x10 x10 1"
                                            << "addi x10 x10 1";
        run_test(processor, program, 3, 6, 6, true);
        unsigned val = ProcessorHandler::get()->getRegisterValue(RegisterFileType::GPR, 10);
        QCOMPARE(val, 5);
    }
}

void tst_reverse::tst_reverse_mem() {
    for (auto processor : {ProcessorID::RV32_SS, ProcessorID::RV32_5S}) {
        QStringList program = QStringList() << ".data"
                                            << "a: .word 42"
                                            << ".text"
                                            << "la a0 a"
                                            << "lw a1 0 a0"
                                            << "addi a1 a1 1"
                                            << "sw a1 0 a0"
                                            << "lw x10 0 a0";
        run_test(processor, program, 3, 10, 10, true);
        unsigned val = ProcessorHandler::get()->getRegisterValue(RegisterFileType::GPR, 10);
        QCOMPARE(val, 43);
    }
}

QTEST_MAIN(tst_reverse)
#include "tst_reverse.moc"
