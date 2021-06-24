#include "processorhandler.h"

#include "processorregistry.h"
#include "processors/interface/ripesprocessor.h"
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
    // Contruct the default processor
    if (RipesSettings::value(RIPES_SETTING_PROCESSOR_ID).isNull()) {
        m_currentID = ProcessorID::RV32_5S;
    } else {
        m_currentID = RipesSettings::value(RIPES_SETTING_PROCESSOR_ID).value<ProcessorID>();

        // Some sanity checking
        m_currentID = m_currentID >= ProcessorID::NUM_PROCESSORS ? ProcessorID::RV32_5S : m_currentID;
    }
    _selectProcessor(m_currentID, ProcessorRegistry::getDescription(m_currentID).isa->supportedExtensions(),
                     ProcessorRegistry::getDescription(m_currentID).defaultRegisterVals);

    // The m_procStateChangeTimer limits maximum frequency of which the procStateChangedNonRun is emitted.
    m_procStateChangeTimer.setSingleShot(true);
    m_procStateChangeTimer.setInterval(1000.0 / RipesSettings::value(RIPES_SETTING_UIUPDATEPS).toUInt());
    connect(RipesSettings::getObserver(RIPES_SETTING_UIUPDATEPS), &SettingObserver::modified, [=] {
        m_procStateChangeTimer.setInterval(1000.0 / RipesSettings::value(RIPES_SETTING_UIUPDATEPS).toUInt());
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

    // Connect relevant settings changes to VSRTL
    connect(RipesSettings::getObserver(RIPES_SETTING_REWINDSTACKSIZE), &SettingObserver::modified,
            [=](const auto& size) { m_currentProcessor->setMaxReverseCycles(size.toUInt()); });
    // Update VSRTL reverse stack size to reflect current settings
    m_currentProcessor->setMaxReverseCycles(RipesSettings::value(RIPES_SETTING_REWINDSTACKSIZE).toUInt());

    m_syscallManager = std::make_unique<RISCVSyscallManager>();
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

const vsrtl::core::AddressSpace& ProcessorHandler::_getRegisters() const {
    return m_currentProcessor->getArchRegisters();
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

void ProcessorHandler::_run() {
    ProcessorStatusManager::setStatus("Running...");
    emit runStarted();
    /** We create a cycleFunctor for running the design which will stop further running of the design when:
     * - The user has stopped running the processor (m_stopRunningFlag)
     * - the processor has finished executing
     * - the processor has hit a breakpoint
     */
    const auto& cycleFunctor = [=] {
        bool stopRunning = m_stopRunningFlag;
        _checkValidExecutionRange();
        stopRunning |= _checkBreakpoint() || m_currentProcessor->finished() || m_stopRunningFlag;

        if (stopRunning) {
            m_vsrtlWidget->stop();
        }
    };

    // Start running through the VSRTL Widget interface
    connect(&m_runWatcher, &QFutureWatcher<void>::finished, [=] {
        emit runFinished();
        _triggerProcStateChangeTimer();
    });
    connect(&m_runWatcher, &QFutureWatcher<void>::finished, [=] { ProcessorStatusManager::clearStatus(); });

    m_runWatcher.setFuture(m_vsrtlWidget->run(cycleFunctor));
}

void ProcessorHandler::_setBreakpoint(const AInt address, bool enabled) {
    if (enabled && _isExecutableAddress(address)) {
        m_breakpoints.insert(address);
    } else {
        m_breakpoints.erase(address);
    }
}

void ProcessorHandler::_loadProcessorToWidget(vsrtl::VSRTLWidget* widget) {
    m_vsrtlWidget = widget;

    // Currently, only VSRTL processors can be visualized
    if (auto* vsrtlProcessor = dynamic_cast<RipesVSRTLProcessor*>(m_currentProcessor.get())) {
        widget->setDesign(vsrtlProcessor);
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

void ProcessorHandler::_selectProcessor(const ProcessorID& id, const QStringList& extensions,
                                        RegisterInitialization setup) {
    m_currentID = id;
    RipesSettings::setValue(RIPES_SETTING_PROCESSOR_ID, id);

    // Keep current program if the ISA between the two processors are identical
    const bool keepProgram = m_currentProcessor && (m_currentProcessor->implementsISA()->eq(
                                                       ProcessorRegistry::getDescription(id).isa, extensions));

    // Processor initializations
    m_currentProcessor = ProcessorRegistry::constructProcessor(m_currentID, extensions);
    m_currentProcessor->isExecutableAddress = [=](AInt address) { return _isExecutableAddress(address); };

    // Syscall handling initialization
    m_currentProcessor->handleSysCall = [=] { syscallTrap(); };

    // Reset request handling
    connect(RipesSettings::getObserver(RIPES_GLOBALSIGNAL_REQRESET), &SettingObserver::modified,
            [=] { ProcessorHandler::get()->getProcessorNonConst()->resetProcessor(); });

    // Register initializations
    auto& regs = m_currentProcessor->getArchRegisters();
    regs.clearInitializationMemories();
    for (const auto& kv : setup) {
        // Memories are initialized through pointers to byte arrays, so we have to transform the intitial pointer
        // address to the compatible format.
        QByteArray ptrValueBytes;
        auto ptrValue = kv.second;
        for (unsigned i = 0; i < m_currentProcessor->implementsISA()->bytes(); i++) {
            ptrValueBytes.push_back(static_cast<char>(ptrValue & 0xFF));
            ptrValue >>= CHAR_BIT;
        }
        regs.addInitializationMemory(kv.first * 4, ptrValueBytes.data(), m_currentProcessor->implementsISA()->bytes());
    }

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
    m_signalWrappers.push_back(std::unique_ptr<GallantSignalWrapperBase>(new GallantSignalWrapper(
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

    m_signalWrappers.push_back(std::unique_ptr<GallantSignalWrapperBase>(new GallantSignalWrapper(
        this,
        [=] {
            emit processorReset();
            _triggerProcStateChangeTimer();
        },
        m_currentProcessor->processorWasReset)));

    m_signalWrappers.push_back(std::unique_ptr<GallantSignalWrapperBase>(new GallantSignalWrapper(
        this,
        [=] {
            emit processorReversed();
            _triggerProcStateChangeTimer();
        },
        m_currentProcessor->processorWasReversed)));

    emit processorChanged();
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
        return m_currentAssembler
            ->disassemble(m_currentProcessor->getMemory().readMem(addr), m_program.get()->symbols, addr)
            .first;
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

void ProcessorHandler::_checkValidExecutionRange() const {
    const auto pc = m_currentProcessor->nextFetchedAddress();
    unsigned fr = 0;
    if (!_isExecutableAddress(pc)) {
        fr |= RipesProcessor::FinalizeReason::exitedExecutableRegion;
    }
    m_currentProcessor->finalize(fr);
}

void ProcessorHandler::_setRegisterValue(RegisterFileType rfid, const unsigned idx, VInt value) {
    m_currentProcessor->setRegister(rfid, idx, value);
}

VInt ProcessorHandler::_getRegisterValue(RegisterFileType rfid, const unsigned idx) const {
    return m_currentProcessor->getRegister(rfid, idx);
}
}  // namespace Ripes
