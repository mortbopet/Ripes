#include "processorhandler.h"

#include "parser.h"
#include "processorregistry.h"
#include "program.h"
#include "ripessettings.h"
#include "statusmanager.h"

#include "syscall/riscv_syscall.h"

#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>

namespace Ripes {

ProcessorHandler::ProcessorHandler() {
    // Contruct the default processor
    if (RipesSettings::value(RIPES_SETTING_PROCESSOR_ID).isNull()) {
        m_currentID = ProcessorID::RV5S;
    } else {
        m_currentID = RipesSettings::value(RIPES_SETTING_PROCESSOR_ID).value<ProcessorID>();

        // Some sanity checking
        m_currentID = m_currentID >= ProcessorID::NUM_PROCESSORS ? ProcessorID::RV5S : m_currentID;
    }
    selectProcessor(m_currentID, ProcessorRegistry::getDescription(m_currentID).defaultRegisterVals);

    // Connect relevant settings changes to VSRTL
    connect(RipesSettings::getObserver(RIPES_SETTING_REWINDSTACKSIZE), &SettingObserver::modified,
            [=](const auto& size) { m_currentProcessor->setReverseStackSize(size.toUInt()); });
    // Update VSRTL reverse stack size to reflect current settings
    m_currentProcessor->setReverseStackSize(RipesSettings::value(RIPES_SETTING_REWINDSTACKSIZE).toUInt());

    m_syscallManager = std::make_unique<RISCVSyscallManager>();
}

void ProcessorHandler::loadProgram(std::shared_ptr<Program> p) {
    // Stop any currently executing simulation
    stopRun();

    auto* textSection = p->getSection(TEXT_SECTION_NAME);
    if (!textSection)
        return;

    auto& mem = m_currentProcessor->getMemory();

    m_program = p;
    // Memory initializations
    mem.clear();
    for (const auto& seg : p->sections) {
        static_assert(sizeof(char) == sizeof(uint8_t));
        mem.getInitSas().insertSegment(seg.address, reinterpret_cast<const uint8_t*>(seg.data.data()), seg.data.size());
    }

    m_currentProcessor->setPCInitialValue(p->entryPoint);

    const auto textStart = textSection->address;
    const auto textEnd = textSection->address + textSection->data.length();

    // Update breakpoints to stay within the loaded program range
    std::vector<uint32_t> bpsToRemove;
    for (const auto& bp : m_breakpoints) {
        if ((bp < textStart) || (bp >= textEnd)) {
            bpsToRemove.push_back(bp);
        }
    }
    for (const auto& bp : bpsToRemove) {
        m_breakpoints.erase(bp);
    }

    emit reqProcessorReset();
}

void ProcessorHandler::writeMem(uint32_t address, uint32_t value, int size) {
    m_currentProcessor->getMemory().writeValue(address, value, size);
}

const vsrtl::core::AddressSpace& ProcessorHandler::getMemory() const {
    return m_currentProcessor->getMemory();
}

const vsrtl::core::RVMemory<RV_REG_WIDTH, RV_REG_WIDTH>* ProcessorHandler::getDataMemory() const {
    return dynamic_cast<const vsrtl::core::RVMemory<RV_REG_WIDTH, RV_REG_WIDTH>*>(m_currentProcessor->getDataMemory());
}
const vsrtl::core::ROM<RV_REG_WIDTH, RV_INSTR_WIDTH>* ProcessorHandler::getInstrMemory() const {
    return dynamic_cast<const vsrtl::core::ROM<RV_REG_WIDTH, RV_INSTR_WIDTH>*>(m_currentProcessor->getInstrMemory());
}

const vsrtl::core::AddressSpace& ProcessorHandler::getRegisters() const {
    return m_currentProcessor->getArchRegisters();
}

void ProcessorHandler::run() {
    ProcessorStatusManager::setStatus("Running...");
    emit runStarted();
    /** We create a cycleFunctor for running the design which will stop further running of the design when:
     * - The user has stopped running the processor (m_stopRunningFlag)
     * - the processor has finished executing
     * - the processor has hit a breakpoint
     */
    const auto& cycleFunctor = [=] {
        bool stopRunning = m_stopRunningFlag;
        ProcessorHandler::get()->checkValidExecutionRange();
        stopRunning |=
            ProcessorHandler::get()->checkBreakpoint() || m_currentProcessor->finished() || m_stopRunningFlag;

        if (stopRunning) {
            m_vsrtlWidget->stop();
        }
    };

    // Start running through the VSRTL Widget interface
    connect(&m_runWatcher, &QFutureWatcher<void>::finished, this, &ProcessorHandler::runFinished);
    connect(&m_runWatcher, &QFutureWatcher<void>::finished, [=] { ProcessorStatusManager::clearStatus(); });

    m_runWatcher.setFuture(m_vsrtlWidget->run(cycleFunctor));
}

void ProcessorHandler::setBreakpoint(const uint32_t address, bool enabled) {
    if (enabled && isExecutableAddress(address)) {
        m_breakpoints.insert(address);
    } else {
        m_breakpoints.erase(address);
    }
}

void ProcessorHandler::loadProcessorToWidget(vsrtl::VSRTLWidget* widget) {
    m_vsrtlWidget = widget;
    widget->setDesign(m_currentProcessor.get());
}

bool ProcessorHandler::hasBreakpoint(const uint32_t address) const {
    return m_breakpoints.count(address);
}

bool ProcessorHandler::checkBreakpoint() {
    const auto pc = m_currentProcessor->getPcForStage(0);
    return m_breakpoints.count(pc);
}

void ProcessorHandler::toggleBreakpoint(const uint32_t address) {
    setBreakpoint(address, !hasBreakpoint(address));
}

void ProcessorHandler::clearBreakpoints() {
    m_breakpoints.clear();
}

void ProcessorHandler::selectProcessor(const ProcessorID& id, RegisterInitialization setup) {
    m_program = nullptr;
    m_currentID = id;
    RipesSettings::setValue(RIPES_SETTING_PROCESSOR_ID, id);

    // Processor initializations
    m_currentProcessor = ProcessorRegistry::constructProcessor(m_currentID);
    m_currentProcessor->isExecutableAddress = [=](uint32_t address) { return isExecutableAddress(address); };

    // Syscall handling initialization
    m_currentProcessor->handleSysCall.Connect(this, &ProcessorHandler::asyncTrap);

    // Register initializations
    auto& regs = m_currentProcessor->getArchRegisters();
    regs.clear();
    for (const auto& kv : setup) {
        const uint32_t regAddr = kv.first << ceillog2(m_currentProcessor->implementsISA()->bytes());
        const uint32_t regValue = kv.second;
        regs.getInitSas().writeValue(regAddr, regValue);
    }

    m_currentProcessor->verifyAndInitialize();

    // Processor loaded. Request for the currently assembled program to be loaded into the processor
    emit reqReloadProgram();
}

int ProcessorHandler::getCurrentProgramSize() const {
    if (m_program) {
        const auto* textSection = m_program->getSection(TEXT_SECTION_NAME);
        if (textSection)
            return textSection->data.length();
    }

    return 0;
}

unsigned long ProcessorHandler::getTextStart() const {
    if (m_program) {
        const auto* textSection = m_program->getSection(TEXT_SECTION_NAME);
        if (textSection)
            return textSection->address;
    }

    return 0;
}

QString ProcessorHandler::parseInstrAt(const uint32_t addr) const {
    if (m_program) {
        return Parser::getParser()->disassemble(m_program, m_currentProcessor->getMemory().readValue<uint32_t>(addr),
                                                addr);
    } else {
        return QString();
    }
}

void ProcessorHandler::asyncTrap() {
    auto futureWatcher = QFutureWatcher<bool>();
    futureWatcher.setFuture(QtConcurrent::run([=] {
        const unsigned int function = m_currentProcessor->getRegister(currentISA()->syscallReg());
        return m_syscallManager->execute(function);
    }));

    futureWatcher.waitForFinished();
    if (!futureWatcher.result()) {
        // Syscall handling failed, stop running processor
        setStopRunFlag();
    }
}

void ProcessorHandler::checkProcessorFinished() {
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

void ProcessorHandler::stopRun() {
    setStopRunFlag();
    m_runWatcher.waitForFinished();
    m_stopRunningFlag = false;
    SystemIO::abortSyscall(false);
}

bool ProcessorHandler::isExecutableAddress(uint32_t address) const {
    if (m_program) {
        if (auto* textSection = m_program->getSection(TEXT_SECTION_NAME)) {
            const auto textStart = textSection->address;
            const auto textEnd = textSection->address + textSection->data.length();
            return textStart <= address && address < textEnd;
        }
    }
    return false;
}

void ProcessorHandler::checkValidExecutionRange() const {
    const auto pc = m_currentProcessor->nextFetchedAddress();
    FinalizeReason fr;
    fr.exitedExecutableRegion = !isExecutableAddress(pc);
    m_currentProcessor->finalize(fr);
}

void ProcessorHandler::setRegisterValue(const unsigned idx, uint32_t value) {
    m_currentProcessor->setRegister(idx, value);
}

uint32_t ProcessorHandler::getRegisterValue(const unsigned idx) const {
    return m_currentProcessor->getRegister(idx);
}
}  // namespace Ripes
