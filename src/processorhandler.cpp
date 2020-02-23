#include "processorhandler.h"

#include "parser.h"
#include "processorregistry.h"
#include "program.h"

#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>

namespace Ripes {

ProcessorHandler::ProcessorHandler() {
    // Contruct the default processor
    selectProcessor(m_currentID, ProcessorRegistry::getDescription(m_currentID).defaultRegisterVals);
}

void ProcessorHandler::loadProgram(const Program* p) {
    // Stop any currently executing simulation
    stop();

    auto* textSection = p->getSection(TEXT_SECTION_NAME);
    if (!textSection)
        return;

    auto& mem = m_currentProcessor->getMemory();

    m_program = p;
    // Memory initializations
    mem.clearInitializationMemories();
    for (const auto& seg : p->sections) {
        mem.addInitializationMemory(seg.address, seg.data.data(), seg.data.length());
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

const vsrtl::core::SparseArray& ProcessorHandler::getMemory() const {
    return m_currentProcessor->getMemory();
}

const vsrtl::core::SparseArray& ProcessorHandler::getRegisters() const {
    return m_currentProcessor->getArchRegisters();
}

void ProcessorHandler::run() {
    auto future = QtConcurrent::run([=] {
        m_currentProcessor->setEnableSignals(false);
        bool stopRunning = m_stopRunningFlag;
        while (!stopRunning) {
            m_currentProcessor->clock();
            ProcessorHandler::get()->checkValidExecutionRange();
            stopRunning |=
                ProcessorHandler::get()->checkBreakpoint() || m_currentProcessor->finished() || m_stopRunningFlag;
        }
        m_currentProcessor->setEnableSignals(true);
    });
    m_runWatcher.setFuture(future);
    connect(&m_runWatcher, &QFutureWatcher<void>::finished, this, &ProcessorHandler::runFinished);
}

void ProcessorHandler::setBreakpoint(const uint32_t address, bool enabled) {
    if (enabled) {
        m_breakpoints.insert(address);
    } else if (m_breakpoints.count(address)) {
        m_breakpoints.erase(address);
    }
}

void ProcessorHandler::loadProcessorToWidget(vsrtl::VSRTLWidget* widget) {
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

    // Processor initializations
    m_currentProcessor = ProcessorRegistry::constructProcessor(m_currentID);
    m_currentProcessor->handleSysCall.Connect(this, &ProcessorHandler::handleSysCall);
    m_currentProcessor->isExecutableAddress = [=](uint32_t address) { return isExecutableAddress(address); };

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
        regs.addInitializationMemory(kv.first << ceillog2(m_currentProcessor->implementsISA()->bytes()),
                                     ptrValueBytes.data(), m_currentProcessor->implementsISA()->bytes());
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
        return Parser::getParser()->disassemble(*m_program, m_currentProcessor->getMemory().readMem(addr), addr);
    } else {
        return QString();
    }
}

void ProcessorHandler::handleSysCall() {
    const unsigned int arg = m_currentProcessor->getRegister(17);
    const auto val = m_currentProcessor->getRegister(10);
    switch (arg) {
        case SysCall::None:
            return;
        case SysCall::PrintInt: {
            emit print(QString::number(static_cast<int>(val)));
            return;
        }
        case SysCall::PrintFloat: {
            auto* v_f = reinterpret_cast<const float*>(&val);
            emit print(QString::number(static_cast<double>(*v_f)));
            return;
        }
        case SysCall::PrintStr: {
            QByteArray string;
            char byte;
            unsigned int address = val;
            do {
                byte = static_cast<char>(m_currentProcessor->getMemory().readMem(address++) & 0xFF);
                string.append(byte);
            } while (byte != '\0');
            emit print(QString::fromUtf8(string));
            return;
        }
        case SysCall::Exit2:
        case SysCall::Exit: {
            FinalizeReason fr;
            fr.exitSyscall = true;
            m_currentProcessor->finalize(fr);
            return;
        }
        case SysCall::PrintChar: {
            QString ch = QChar(val);
            emit print(ch);
            break;
        }
        case SysCall::PrintIntHex: {
            emit print("0x" + QString::number(val, 16).rightJustified(currentISA()->bytes(), '0'));
            return;
        }
        case SysCall::PrintIntBinary: {
            emit print("0b" + QString::number(val, 2).rightJustified(currentISA()->bits(), '0'));
            return;
        }
        case SysCall::PrintIntUnsigned: {
            emit print(QString::number(static_cast<unsigned>(val)));
            return;
        }
        default: {
            QMessageBox::warning(nullptr, "Error",
                                 "Unknown system call argument in register a0: " + QString::number(arg));
            return;
        }
    }
}

void ProcessorHandler::checkProcessorFinished() {
    if (m_currentProcessor->finished())
        emit exit();
}

void ProcessorHandler::stop() {
    if (m_runWatcher.isRunning())
        m_stopRunningFlag = true;
    m_runWatcher.waitForFinished();
    m_stopRunningFlag = false;
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
