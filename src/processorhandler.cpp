#include "processorhandler.h"

#include "processorregistry.h"
#include "processors/ripesvsrtlprocessor.h"
#include "ripessettings.h"
#include "statusmanager.h"

#include "assembler/program.h"
#include "assembler/rv32i_assembler.h"
#include "assembler/rv64i_assembler.h"

#include "syscall/riscv_syscall.h"

#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>

namespace Ripes {

ProcessorHandler::ProcessorHandler() {
    m_constructing = true;

    // Contruct the default processor
    if (RipesSettings::value(RIPES_SETTING_PROCESSOR_ID).isNull()) {
        m_currentID = ProcessorID::RV32_5S;
    } else {
        m_currentID = RipesSettings::value(RIPES_SETTING_PROCESSOR_ID).value<ProcessorID>();

        // Some sanity checking
        m_currentID = m_currentID >= ProcessorID::NUM_PROCESSORS ? ProcessorID::RV32_5S : m_currentID;
    }
    _selectProcessor(m_currentID, ProcessorRegistry::getDescription(m_currentID).isaInfo().isa->supportedExtensions(),
                     ProcessorRegistry::getDescription(m_currentID).defaultRegisterVals);

    // The m_procStateChangeTimer limits maximum frequency of which the procStateChangedNonRun is emitted.
    m_procStateChangeTimer.setSingleShot(true);
    m_procStateChangeTimer.setInterval(1000.0 / RipesSettings::value(RIPES_SETTING_UIUPDATEPS).toInt());
    connect(RipesSettings::getObserver(RIPES_SETTING_UIUPDATEPS), &SettingObserver::modified, this, [=] {
        m_procStateChangeTimer.setInterval(1000.0 / RipesSettings::value(RIPES_SETTING_UIUPDATEPS).toInt());
    });

    connect(&m_procStateChangeTimer, &QTimer::timeout, this, [=] {
        emit procStateChangedNonRun();
        m_enqueueStateChangeLock.lock();
        if (m_enqueueStateChangeSignal) {
            m_enqueueStateChangeSignal = false;
            m_procStateChangeTimer.start();
        }
        m_enqueueStateChangeLock.unlock();
    });

    // Connect the runwatcher finished signals
    connect(&m_runWatcher, &QFutureWatcher<void>::finished, this, [=] {
        emit runFinished();
        _triggerProcStateChangeTimer();
    });
    connect(&m_runWatcher, &QFutureWatcher<void>::finished, this, [=] { ProcessorStatusManager::clearStatus(); });

    // Connect relevant settings changes to VSRTL
    connect(RipesSettings::getObserver(RIPES_SETTING_REWINDSTACKSIZE), &SettingObserver::modified,
            [=](const auto& size) { m_currentProcessor->setMaxReverseCycles(size.toUInt()); });

    // Update VSRTL reverse stack size to reflect current settings
    m_currentProcessor->setMaxReverseCycles(RipesSettings::value(RIPES_SETTING_REWINDSTACKSIZE).toInt());

    // Reset request handling
    connect(RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET), &SettingObserver::modified, this,
            &ProcessorHandler::_reset);

    m_syscallManager = std::make_unique<RISCVSyscallManager>();
    m_constructing = false;
}

bool ProcessorHandler::isVSRTLProcessor() {
    return static_cast<bool>(dynamic_cast<const RipesVSRTLProcessor*>(getProcessor()));
}

void ProcessorHandler::_loadProgram(const std::shared_ptr<Program>& p) {
    // Stop any currently executing simulation
    stopRun();

    auto* textSection = p->getSection(TEXT_SECTION_NAME);
    if (!textSection)
        return;

    auto& mem = m_currentProcessor->getMemory();

    m_program = p;
    // Memory initializations
    mem.clearInitializationMemories();
    for (const auto& seg : p->sections) {
        mem.addInitializationMemory(seg.second.address, seg.second.data.data(), seg.second.data.length());
    }

    m_currentProcessor->setPCInitialValue(p->entryPoint);

    const auto textStart = textSection->address;
    const auto textEnd = textSection->address + textSection->data.length();

    // Update breakpoints to stay within the loaded program range
    std::vector<AInt> bpsToRemove;
    for (const auto& bp : m_breakpoints) {
        if ((bp < textStart) || (bp >= textEnd)) {
            bpsToRemove.push_back(bp);
        }
    }
    for (const auto& bp : bpsToRemove) {
        m_breakpoints.erase(bp);
    }

    RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET)->trigger();
    emit programChanged();
}

void ProcessorHandler::_writeMem(AInt address, VInt value, int size) {
    m_currentProcessor->getMemory().writeMem(address, value, size);
}

vsrtl::core::AddressSpaceMM& ProcessorHandler::_getMemory() {
    return m_currentProcessor->getMemory();
}

void ProcessorHandler::_triggerProcStateChangeTimer() {
    m_enqueueStateChangeLock.lock();
    if (!m_procStateChangeTimer.isActive()) {
        m_enqueueStateChangeSignal = false;
        m_procStateChangeTimer.start();
    } else {
        m_enqueueStateChangeSignal = true;
    }
    m_enqueueStateChangeLock.unlock();
}

class ProcessorClocker : public QRunnable {
public:
    explicit ProcessorClocker(bool& finished) : m_finished(finished) {}
    void run() override {
        ProcessorHandler::getProcessorNonConst()->clock();
        ProcessorHandler::checkProcessorFinished();
        if (ProcessorHandler::checkBreakpoint()) {
            ProcessorHandler::stopRun();
        }
        m_finished = true;
    }

private:
    bool& m_finished;
};

void ProcessorHandler::_clock() {
    if (m_clockFinished) {
        m_clockFinished = false;
        QThreadPool::globalInstance()->start(new ProcessorClocker(m_clockFinished));
    }
}

void ProcessorHandler::_run() {
    ProcessorStatusManager::setStatusTimed("Running...");
    emit runStarted();

    // Start running through the VSRTL Widget interface
    m_runWatcher.setFuture(QtConcurrent::run([=] {
        auto* vsrtl_proc = dynamic_cast<vsrtl::SimDesign*>(m_currentProcessor.get());

        if (vsrtl_proc) {
            vsrtl_proc->setEnableSignals(false);
        }

        while (!(_checkBreakpoint() || m_currentProcessor->finished() || m_stopRunningFlag)) {
            m_currentProcessor->clock();
        }

        if (vsrtl_proc) {
            vsrtl_proc->setEnableSignals(true);
        }
        emit runFinished();
    }));
}

void ProcessorHandler::_setBreakpoint(const AInt address, bool enabled) {
    if (enabled && _isExecutableAddress(address)) {
        m_breakpoints.insert(address);
    } else {
        m_breakpoints.erase(address);
    }
}

void ProcessorHandler::_loadProcessorToWidget(vsrtl::VSRTLWidget* widget, bool doPlaceAndRoute) {
    m_vsrtlWidget = widget;

    // Currently, only VSRTL processors can be visualized
    if (auto* vsrtlProcessor = dynamic_cast<RipesVSRTLProcessor*>(m_currentProcessor.get())) {
        widget->setDesign(vsrtlProcessor, doPlaceAndRoute);
    }
}

bool ProcessorHandler::_hasBreakpoint(const AInt address) const {
    return m_breakpoints.count(address);
}

bool ProcessorHandler::_checkBreakpoint() {
    for (const auto& stage : m_currentProcessor->breakpointTriggeringStages()) {
        const auto it = m_breakpoints.find(m_currentProcessor->getPcForStage(stage));
        if (it != m_breakpoints.end()) {
            return true;
        }
    }
    return false;
}

void ProcessorHandler::_toggleBreakpoint(const AInt address) {
    _setBreakpoint(address, !hasBreakpoint(address));
}

void ProcessorHandler::_clearBreakpoints() {
    m_breakpoints.clear();
}

void ProcessorHandler::createAssemblerForCurrentISA() {
    const auto& ISA = _currentISA();

    if (auto* rv32isa = dynamic_cast<const ISAInfo<ISA::RV32I>*>(ISA)) {
        m_currentAssembler = std::make_shared<Assembler::RV32I_Assembler>(rv32isa);
    } else if (auto* rv64isa = dynamic_cast<const ISAInfo<ISA::RV64I>*>(ISA)) {
        m_currentAssembler = std::make_shared<Assembler::RV64I_Assembler>(rv64isa);
    } else {
        Q_UNREACHABLE();
    }
}

void ProcessorHandler::_reset() {
    if (m_constructing) {
        return;
    }

    getProcessorNonConst()->resetProcessor();

    // Rewrite register initializations
    for (const auto& kv : m_currentRegInits) {
        _setRegisterValue(RegisterFileType::GPR, kv.first, kv.second);
    }
    // Forcing memory values doesn't necessarily mean that the processor will notify that its state changed. Manually
    // trigger a state change signal, to ensure this.
    emit procStateChangedNonRun();
}

void ProcessorHandler::_selectProcessor(const ProcessorID& id, const QStringList& extensions,
                                        RegisterInitialization setup) {
    m_currentID = id;
    m_currentRegInits = setup;
    RipesSettings::setValue(RIPES_SETTING_PROCESSOR_ID, id);

    // Keep current program if the ISA between the two processors are identical
    const bool keepProgram =
        m_currentProcessor && (m_currentProcessor->implementsISA()->eq(
                                  ProcessorRegistry::getDescription(id).isaInfo().isa.get(), extensions));

    // Processor initializations
    m_currentProcessor = ProcessorRegistry::constructProcessor(m_currentID, extensions);
    m_currentProcessor->isExecutableAddress = [=](AInt address) { return _isExecutableAddress(address); };

    // Syscall handling initialization
    m_currentProcessor->trapHandler = [=] { syscallTrap(); };

    m_currentProcessor->postConstruct();
    createAssemblerForCurrentISA();

    if (keepProgram && m_program) {
        loadProgram(m_program);
    } else {
        m_program = nullptr;
        emit programChanged();
    }

    // Connect wrappers for making processor signal emissions thread safe.
    m_signalWrappers.clear();
    m_signalWrappers.push_back(std::unique_ptr<vsrtl::GallantSignalWrapperBase>(new vsrtl::GallantSignalWrapper(
        this,
        [=] {
            if (!_isRunning()) {
                emit processorClockedNonRun();
                _triggerProcStateChangeTimer();
            }
        },
        m_currentProcessor->processorWasClocked)));
    // Connect ProcessorHandler::processorClocked since things connected to this signal _must_ be updated _for each_
    // processor cycle, in order. Which would not be possible through processorClockedNonRun, which might be
    // cross-thread and out of order.
    m_currentProcessor->processorWasClocked.Connect(this, &ProcessorHandler::processorClocked);

    m_signalWrappers.push_back(std::unique_ptr<vsrtl::GallantSignalWrapperBase>(new vsrtl::GallantSignalWrapper(
        this,
        [=] {
            emit processorReset();
            _triggerProcStateChangeTimer();
        },
        m_currentProcessor->processorWasReset)));

    m_signalWrappers.push_back(std::unique_ptr<vsrtl::GallantSignalWrapperBase>(new vsrtl::GallantSignalWrapper(
        this,
        [=] {
            emit processorReversed();
            _triggerProcStateChangeTimer();
        },
        m_currentProcessor->processorWasReversed)));

    emit processorChanged();

    // Finally, reset the processor
    RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET)->trigger();
}

int ProcessorHandler::_getCurrentProgramSize() const {
    if (m_program) {
        const auto* textSection = m_program->getSection(TEXT_SECTION_NAME);
        if (textSection)
            return textSection->data.length();
    }

    return 0;
}

AInt ProcessorHandler::_getTextStart() const {
    if (m_program) {
        const auto* textSection = m_program->getSection(TEXT_SECTION_NAME);
        if (textSection)
            return textSection->address;
    }

    return 0;
}

QString ProcessorHandler::_disassembleInstr(const AInt addr) const {
    if (m_program) {
        const unsigned instrBytes = _currentISA()->instrBytes();
        auto disRes = m_currentAssembler->disassemble(m_currentProcessor->getMemory().readMem(addr, instrBytes),
                                                      m_program.get()->symbols, addr);
        return disRes.repr;
    } else {
        return QString();
    }
}

void ProcessorHandler::syscallTrap() {
    auto futureWatcher = QFutureWatcher<bool>();
    futureWatcher.setFuture(QtConcurrent::run([=] {
        const unsigned int function =
            m_currentProcessor->getRegister(RegisterFileType::GPR, _currentISA()->syscallReg());
        return m_syscallManager->execute(function);
    }));

    futureWatcher.waitForFinished();
    if (!futureWatcher.result()) {
        // Syscall handling failed, stop running processor
        setStopRunFlag();
    }
}

bool ProcessorHandler::_isRunning() {
    return !m_runWatcher.isFinished();
}

void ProcessorHandler::_checkProcessorFinished() {
    if (m_currentProcessor->finished())
        emit exit();
}

void ProcessorHandler::setStopRunFlag() {
    emit stopping();
    if (m_runWatcher.isRunning()) {
        m_stopRunningFlag = true;
        // We might be currently trapping for user I/O. Signal to abort the trap, in this avoiding a deadlock.
        SystemIO::abortSyscall(true);
    }
}

void ProcessorHandler::_stopRun() {
    setStopRunFlag();
    m_runWatcher.waitForFinished();
    m_stopRunningFlag = false;
    SystemIO::abortSyscall(false);
}

bool ProcessorHandler::_isExecutableAddress(AInt address) const {
    if (m_program) {
        if (auto* textSection = m_program->getSection(TEXT_SECTION_NAME)) {
            const auto textStart = textSection->address;
            const auto textEnd = textSection->address + textSection->data.length();
            return textStart <= address && address < textEnd;
        }
    }
    return false;
}

void ProcessorHandler::_setRegisterValue(RegisterFileType rfid, const unsigned idx, VInt value) {
    m_currentProcessor->setRegister(rfid, idx, value);
}

VInt ProcessorHandler::_getRegisterValue(RegisterFileType rfid, const unsigned idx) const {
    return m_currentProcessor->getRegister(rfid, idx);
}
}  // namespace Ripes
