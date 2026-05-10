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

// This test ensures that the stalling works correctly.

class tst_stall : public QObject {
  Q_OBJECT

private slots:
  void run_test(ProcessorID proc, VariationID variation,
                const QStringList &program);
  void tst_no_stall_load_ex_hazard_rs1();
  void tst_no_stall_load_ex_hazard_rs2();
  void tst_no_stall_load_ex_hazard_x0();
};

void tst_stall::run_test(ProcessorID procid, VariationID variation,
                         const QStringList &program) {
  ProcessorHandler::selectProcessor(procid, variation, RV_ExtensionSet());
  RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET)->trigger();
  ProcessorHandler::get()->getProcessorNonConst()->trapHandler = [this] {};

  auto loader = new ProgramLoader();
  loader->loadTest(program.join("\n"));
  auto proc = ProcessorHandler::get()->getProcessorNonConst();

  // Step until finished
  while (!proc->finished() && proc->getCycleCount() < 1000)
    proc->clock();

  if (!proc->finished())
    QFAIL("Execution never finished");

  return;
}

void tst_stall::tst_no_stall_load_ex_hazard_rs1() {
  for (auto variation :
       {Variations::RV_5S::RV32I_FU_HU, Variations::RV_5S::RV64I_FU_HU}) {
    QStringList program = QStringList() << ".data"
                                        << "A: .word 5"
                                        << ".text"
                                        << "la a0, A"
                                        << "lw x14, 0(a0)"
                                        << "lui x6, 0x70";
    run_test(ProcessorID::RV_5S, variation, program);
    unsigned val = ProcessorHandler::get()->getProcessor()->getCycleCount();
    QCOMPARE(val, 8);
  }
}

void tst_stall::tst_no_stall_load_ex_hazard_rs2() {
  for (auto variation :
       {Variations::RV_5S::RV32I_FU_HU, Variations::RV_5S::RV64I_FU_HU}) {
    QStringList program = QStringList() << ".data"
                                        << "A: .word 5"
                                        << ".text"
                                        << "la a0, A"
                                        << "lw x27, 0(a0)"
                                        << "lw t0, 27(a0)";
    run_test(ProcessorID::RV_5S, variation, program);
    unsigned val = ProcessorHandler::get()->getProcessor()->getCycleCount();
    QCOMPARE(val, 8);
  }
}

void tst_stall::tst_no_stall_load_ex_hazard_x0() {
  for (auto variation :
       {Variations::RV_5S::RV32I_FU_HU, Variations::RV_5S::RV64I_FU_HU}) {
    QStringList program = QStringList() << ".data"
                                        << "A: .word 5"
                                        << ".text"
                                        << "la a0, A"
                                        << "lw x0, 0(a0)"
                                        << "addi t0, x0, 14";
    run_test(ProcessorID::RV_5S, variation, program);
    unsigned val = ProcessorHandler::get()->getProcessor()->getCycleCount();
    QCOMPARE(val, 8);
  }
}

QTEST_MAIN(tst_stall)
#include "tst_stall.moc"
