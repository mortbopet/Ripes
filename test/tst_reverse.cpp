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

// This test ensures that the 'reverse' feature works across register and memory
// writes.

class tst_reverse : public QObject {
  Q_OBJECT

private slots:
  void run_test(ProcessorID id, VariationID variation, 
                const RV_ExtensionSet &extensions,
                const QStringList &program,
                unsigned rounds, unsigned front, unsigned back,
                bool toFinish);
  void tst_reverse_regs();
  void tst_reverse_fpr();
  void tst_reverse_mem();

  void tst_body_reverse_regs(ProcessorID id, VariationID variation, const RV_ExtensionSet &extensions);
  void tst_body_reverse_fpr(ProcessorID id, VariationID variation, const RV_ExtensionSet &extensions);
  void tst_body_reverse_mem(ProcessorID id, VariationID variation, const RV_ExtensionSet &extensions);
};

using Registers = std::map<int, VInt>;
static Registers dumpRegs() {
  Registers regs;
  for (const auto &regFile :
       ProcessorHandler::get()->currentISA()->regInfos()) {
    for (unsigned i = 0; i < regFile->regCnt(); i++) {
      regs[i] = ProcessorHandler::get()->getProcessor()->getRegister(
          regFile->regFileName(), i);
    }
  }
  return regs;
}

void tst_reverse::run_test(ProcessorID id, VariationID variation, 
                           const RV_ExtensionSet &extensions,
                           const QStringList &program,
                           unsigned rounds, unsigned front, unsigned back,
                           bool toFinish) {
  ProcessorHandler::selectProcessor(id, variation, extensions);
  RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET)->trigger();
  ProcessorHandler::getProcessorNonConst()->trapHandler = [this] {};

  auto loader = new ProgramLoader();
  loader->loadTest(program.join("\n"));
  auto proc = ProcessorHandler::getProcessorNonConst();
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
  tst_body_reverse_regs(ProcessorID::RV_SS, Variations::RV_SS::RV32I, {Extension::M, Extension::C});
  tst_body_reverse_regs(ProcessorID::RV_5S, Variations::RV_5S::RV32I_FU_HU, {Extension::M, Extension::C});
}
void tst_reverse::tst_body_reverse_regs(ProcessorID id, VariationID variation, const RV_ExtensionSet &extensions) {
  QStringList program = QStringList() << ".text"
                                      << "li x10 0"
                                      << "addi x10 x10 1"
                                      << "addi x10 x10 1"
                                      << "addi x10 x10 1"
                                      << "addi x10 x10 1"
                                      << "addi x10 x10 1";
  run_test(id, variation, extensions, program, 3, 6, 6, true);
  unsigned val = ProcessorHandler::getRegisterValue(RVISA::GPR, 10);
  QCOMPARE(val, 5);
}

void tst_reverse::tst_reverse_fpr() {
  tst_body_reverse_fpr(ProcessorID::RV_SS, Variations::RV_SS::RV32F, {Extension::M, Extension::C, Extension::F});
}
void tst_reverse::tst_body_reverse_fpr(ProcessorID id, VariationID variation, const RV_ExtensionSet &extensions) {
  QStringList program = QStringList() << ".text"
                                      << "li x10 1"
                                      << "fcvt.s.wu f1, x10"
                                      << "fcvt.s.wu f10, x0"
                                      << "fadd.s f10, f10, f1"
                                      << "fadd.s f10, f10, f1"
                                      << "fadd.s f10, f10, f1"
                                      << "fadd.s f10, f10, f1"
                                      << "fadd.s f10, f10, f1"
                                      << "fcvt.wu.s x10, f10";
  run_test(id, variation, extensions, program, 3, 9, 9, true);
  unsigned val = ProcessorHandler::getRegisterValue(RVISA::GPR, 10);
  QCOMPARE(val, 5);
}

void tst_reverse::tst_reverse_mem() {
  tst_body_reverse_mem(ProcessorID::RV_SS, Variations::RV_SS::RV32I, {Extension::M, Extension::C});
  tst_body_reverse_mem(ProcessorID::RV_5S, Variations::RV_5S::RV32I_FU_HU, {Extension::M, Extension::C});
}
void tst_reverse::tst_body_reverse_mem(ProcessorID id, VariationID variation, const RV_ExtensionSet &extensions) {
  QStringList program = QStringList() << ".data"
                                      << "a: .word 42"
                                      << ".text"
                                      << "la a0 a"
                                      << "lw a1 0 a0"
                                      << "addi a1 a1 1"
                                      << "sw a1 0 a0"
                                      << "lw x10 0 a0";
  run_test(id, variation, extensions, program, 3, 10, 10, true);
  unsigned val = ProcessorHandler::getRegisterValue(RVISA::GPR, 10);
  QCOMPARE(val, 43);
}

QTEST_MAIN(tst_reverse)
#include "tst_reverse.moc"
