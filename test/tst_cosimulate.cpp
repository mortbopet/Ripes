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

/**
 * Ripes co-simulation
 * For a given test program, executes a reference model (RVSS) to generate a
 * trace of register modifications. Then, the target processor is simulated, and
 * its register trace is compared to the reference trace. From this, we can
 * detect whether (and where) register state divergence occurs, which indicates
 * an error in a processor implementation
 */

using namespace Ripes;
using namespace vsrtl::core;
struct RegisterChange {
  RegisterFileType type;
  unsigned index;
  VInt newValue;
};

// Maximum cycle count
static constexpr unsigned s_maxCycles = 1000000;
using Registers = std::map<int, VInt>;
struct TraceEntry {
  Registers regs;
  unsigned long cycle;
  AInt pc;
};

using Trace = std::vector<TraceEntry>;

// Reference model
// All tests will be compared against the trace generated by this processor
// model.
static constexpr ProcessorID s_referenceModel = ProcessorID::RV32_SS;

// Test selection
// s_testFiles denotes all of the test files that will be included in the
// cosimulation run for each processor.
const QString s_testdir = RISCV32_TEST_DIR;
const std::vector<LoadFileParams> s_testFiles = {
    LoadFileParams{QString(s_testdir + QDir::separator() +
                           "../../examples/assembly/complexMul.s"),
                   SourceType::Assembly, 0, 0},
    LoadFileParams{QString(s_testdir + QDir::separator() +
                           "../../examples/assembly/factorial.s"),
                   SourceType::Assembly, 0, 0},
    LoadFileParams{QString(s_testdir + QDir::separator() +
                           "../../examples/ELF/RanPi-RV32"),
                   SourceType::ExternalELF, 0, 0}};

class tst_Cosimulate : public QObject {
  Q_OBJECT

public:
  explicit tst_Cosimulate() {}

private:
  void cosimulate(const ProcessorID &id, const QStringList &extensions);
  const Trace &generateReferenceTrace(const QStringList &extensions);
  void trapHandler();
  void executeSimulator(Trace &outTrace, const Trace *refTrace = nullptr);
  Registers dumpRegs();
  QString generateErrorReport(const RegisterChange &change,
                              const TraceEntry &lhs,
                              const TraceEntry &rhs) const;
  void loadCurrentTest();

  bool m_stop = false;
  QString m_err;
  LoadFileParams m_currentTest;
  ProgramLoader *m_loader;

  std::map<QString, Trace> m_referenceTraces;

private slots:
  /**
   * PROCESSOR MODELS TO TEST
   * Each of the following functions shall indicate a processor model to
   * co-simulate.
   */
  void testRV6SDual() { cosimulate(ProcessorID::RV32_6S_DUAL, {"M"}); }
  void testRV5S() { cosimulate(ProcessorID::RV32_5S, {"M"}); }
  void testRV5SNoFW() { cosimulate(ProcessorID::RV32_5S_NO_FW, {"M"}); }
};

void tst_Cosimulate::trapHandler() {
  if (auto reg = ProcessorHandler::get()
                     ->getProcessor()
                     ->implementsISA()
                     ->syscallReg();
      reg.has_value()) {
    unsigned status = ProcessorHandler::get()->getProcessor()->getRegister(
        reg->file->regFileType(), reg->index);

    /// @todo: Generalize this by having ISA report exit syscall codes
    if (status == RVABI::SysCall::Exit || status == RVABI::SysCall::Exit2) {
      m_stop = true;
    }
  }
}

Registers tst_Cosimulate::dumpRegs() {
  Registers regs;
  for (const auto &regFile :
       ProcessorHandler::get()->currentISA()->regInfos()) {
    for (unsigned i = 0; i < regFile->regCnt(); i++) {
      regs[i] = ProcessorHandler::get()->getProcessor()->getRegister(
          regFile->regFileType(), i);
    }
  }
  return regs;
}

std::optional<std::vector<int>> regNeq(const Registers &lhs,
                                       const Registers &rhs) {
  std::vector<int> uneqIdxes;
  for (const auto &it : lhs) {
    if (it.second != rhs.at(it.first)) {
      uneqIdxes.push_back(it.first);
    }
  }

  if (uneqIdxes.size() > 0) {
    return {uneqIdxes};
  } else {
    return std::nullopt;
  }
}

std::vector<RegisterChange> registerChange(const Registers &before,
                                           const Registers &after) {
  std::vector<RegisterChange> change;
  for (const auto &regFile :
       ProcessorHandler::get()->currentISA()->regInfos()) {
    for (unsigned i = 0; i < regFile->regCnt(); i++) {
      if (before.at(i) != after.at(i)) {
        change.push_back({regFile->regFileType(), i, after.at(i)});
      }
    }
  }
  return change;
}

QString tst_Cosimulate::generateErrorReport(const RegisterChange &change,
                                            const TraceEntry &lhs,
                                            const TraceEntry &rhs) const {
  QString err;
  err += "\nRegister change discrepancy detected while executing test: " +
         m_currentTest.filepath;
  err += "\nUnexpected change was: ";
  if (auto regInfo =
          ProcessorHandler::get()->currentISA()->regInfo(change.type);
      regInfo.has_value()) {
    err += (*regInfo)->regName(change.index);
  } else {
    err += "x" + QString::number(change.index);
  }
  err += " -> 0x" + QString::number(change.newValue, 16) + "\n";
  err += "\nTest processor state: \t\tPC: 0x" + QString::number(lhs.pc, 16) +
         "\t Cycle #: " + QString::number(lhs.cycle);
  err += "\nReference processor state: \tPC: 0x" + QString::number(rhs.pc, 16) +
         "\t Cycle #: " + QString::number(rhs.cycle);
  err += "\n";

  std::vector<int> idxes = regNeq(lhs.regs, rhs.regs).value();
  for (const auto idx : idxes) {
    err += "Difference in register x" + QString::number(idx) + ":";
    err += "\t expected: 0x" + QString::number(rhs.regs.at(idx), 16) +
           "\tactual: 0x" + QString::number(lhs.regs.at(idx), 16) + "\n";
  }

  return err;
}

/**
 * @brief tst_Cosimulate::executeSimulator
 * Runs the currently loaded simulator on the currently loaded program,
 * generating a register trace while doing so. If @p refTrace is provided, the
 * generated trace is compared to the reference trace and the test fails if a
 * discrepancy is detected.
 */
void tst_Cosimulate::executeSimulator(Trace &trace, const Trace *refTrace) {
  m_loader->loadTest(m_currentTest);

  // Override the ProcessorHandler's ECALL handling. In doing so, we can hook
  // into when the EXIT syscall was executed, to verify whether the correct test
  // value was reached.
  ProcessorHandler::get()->getProcessorNonConst()->trapHandler = [=] {
    trapHandler();
  };

  m_stop = false;
  m_err = QString();
  bool maxCyclesReached = false;
  unsigned cycles = 0;
  trace.push_back(TraceEntry{
      dumpRegs(), cycles,
      ProcessorHandler::get()->getProcessor()->getPcForStage({0, 0})});

  decltype(refTrace->begin()) cmpRegState;
  if (refTrace) {
    cmpRegState = refTrace->begin();
    cmpRegState++; // skip initial state
  }

  Registers preRegs = dumpRegs();

  do {
    ProcessorHandler::get()->getProcessorNonConst()->clock();
    cycles++;

    Registers regs = dumpRegs();
    auto regChange = registerChange(preRegs, regs);
    // Detect change in current register state
    if (regNeq(regs, trace.rbegin()->regs)) {
      trace.push_back(TraceEntry{
          dumpRegs(), cycles,
          ProcessorHandler::get()->getProcessor()->getPcForStage({0, 0})});

      // Check whether change corresponds to expected change in comparison
      // trace. regChange might contain multiple register changes (for
      // processors that can commit >1 instruction per cycle). So we try to
      // locate each of the changes in the next expected register set
      // (cmpRegStatePtr), until we have accounted for all register changes. If
      // we incremented the expected register set, but did not find an
      // accompanying register change, then we've reached a point of divergence.
      if (refTrace != nullptr) {
        while (regChange.size() > 0) {
          bool foundChange = false;
          for (auto changeIt = regChange.begin(); changeIt != regChange.end();
               changeIt++) {
            if (cmpRegState->regs.at(changeIt->index) == changeIt->newValue) {
              regChange.erase(changeIt);
              cmpRegState++;
              foundChange = true;
              break;
            }
          }
          if (!foundChange) {
            const QString err = generateErrorReport(
                *regChange.begin(), *trace.rbegin(), *cmpRegState);
            QFAIL(err.toStdString().c_str());
          }
        }
      }
    }
    preRegs = regs;

    maxCyclesReached = cycles >= s_maxCycles;
    m_stop |=
        maxCyclesReached || ProcessorHandler::get()->getProcessor()->finished();
  } while (!m_stop);

  if (maxCyclesReached) {
    QFAIL("Maximum cycles reached");
  }
}

/**
 * @brief tst_Cosimulate::generateTrace
 * Executes program @p spProgram on the single-cycle processor model, to
 * generate a reference trace.
 */
const Trace &
tst_Cosimulate::generateReferenceTrace(const QStringList &extensions) {
  auto refTrace = m_referenceTraces.find(m_currentTest.filepath);
  if (refTrace == m_referenceTraces.end()) {
    std::cout << "First time running test; generating reference trace..."
              << std::endl;
    ProcessorHandler::get()->selectProcessor(s_referenceModel, extensions);
    Trace trace;
    executeSimulator(trace);
    m_referenceTraces[m_currentTest.filepath] = trace;
    refTrace = m_referenceTraces.find(m_currentTest.filepath);
  }
  return refTrace->second;
}

/**
 * @brief tst_Cosimulate::cosimulate
 * Cosimulate a given processor with a reference model.
 * The cosimulation is quite inefficient, since we don't concurrently execute
 * two processors but rather generate a reference trace from a reference model,
 * and then execute a test model, generate a trace for this, and comprare this
 * trace with the reference trace. The reason for this is, that only a single
 * processor model can be instantiated at once in Ripes, due to the static
 * nature of the ProcessorHandler.
 */
void tst_Cosimulate::cosimulate(const ProcessorID &id,
                                const QStringList &extensions) {
  m_loader = new ProgramLoader();
  for (const auto &test : s_testFiles) {
    m_currentTest = test;
    std::cout << test.filepath.toStdString() << std::endl;
    auto referenceTrace = generateReferenceTrace(extensions);
    ProcessorHandler::get()->selectProcessor(id, extensions);
    RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET)->trigger();

    Trace trace;
    std::cout << "Cosimulating... " << std::flush;
    executeSimulator(trace, &referenceTrace);
    std::cout << "PASS!\n" << std::endl;
  }
}

QTEST_MAIN(tst_Cosimulate)
#include "tst_cosimulate.moc"
