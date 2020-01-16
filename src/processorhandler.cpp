#include "processorhandler.h"

#include "parser.h"
#include "processorregistry.h"
#include "program.h"

#include <QMessageBox>

namespace Ripes {

ProcessorHandler::ProcessorHandler() {
    // Contruct the default processor
    selectProcessor(m_currentSetup);
}

void ProcessorHandler::loadProgram(const Program* p) {
    auto& mem = m_currentProcessor->getMemory();

    m_program = p;
    // Memory initializations
    mem.clearInitializationMemories();
    mem.addInitializationMemory(p->text.first, p->text.second.data(), p->text.second.length());
    for (const auto& seg : p->others) {
        mem.addInitializationMemory(seg.first, seg.second.data(), seg.second.length());
    }

    m_currentProcessor->setPCInitialValue(p->entryPoint);

    // Set the valid execution range to be contained within the .text segment.
    m_validExecutionRange = {p->text.first, p->text.first + p->text.second.length()};

    // Update breakpoints to stay within the loaded program range
    std::vector<uint32_t> bpsToRemove;
    for (const auto& bp : m_breakpoints) {
        if (bp < m_validExecutionRange.first | bp >= m_validExecutionRange.second) {
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
    return m_currentProcessor->getRegisters();
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

void ProcessorHandler::checkBreakpoint() {
    const auto pc = m_currentProcessor->pcForStage(0);
    if (m_breakpoints.count(pc)) {
        emit hitBreakpoint(pc);
    }
}

void ProcessorHandler::toggleBreakpoint(const uint32_t address) {
    setBreakpoint(address, !hasBreakpoint(address));
}

void ProcessorHandler::clearBreakpoints() {
    m_breakpoints.clear();
}

void ProcessorHandler::selectProcessor(const ProcessorSetup& setup) {
    m_currentSetup = setup;

    // Processor initializations
    m_currentProcessor = ProcessorRegistry::constructProcessor(m_currentSetup.id);
    m_currentProcessor->handleSysCall.Connect(this, &ProcessorHandler::handleSysCall);
    m_currentProcessor->finished.Connect(this, &ProcessorHandler::processorFinished);

    // Register initializations
    auto& regs = m_currentProcessor->getRegisters();
    regs.clearInitializationMemories();
    for (const auto& seg : m_currentSetup.segmentPtrs) {
        // Memories are initialized through pointers to byte arrays, so we have to transform the intitial pointer
        // address to the compatible format.
        QByteArray ptrValueBytes;
        auto ptrValue = seg.second;
        for (int i = 0; i < m_currentProcessor->implementsISA()->bytes(); i++) {
            ptrValueBytes.push_back(static_cast<char>(ptrValue & 0xFF));
            ptrValue >>= CHAR_BIT;
        }
        switch (seg.first) {
            case ProgramSegment::Stack: {
                const int sp_idx = m_currentProcessor->implementsISA()->spReg();
                if (sp_idx >= 0) {
                    regs.addInitializationMemory(sp_idx << ceillog2(m_currentProcessor->implementsISA()->bytes()),
                                                 ptrValueBytes.data(), m_currentProcessor->implementsISA()->bytes());
                }
                break;
            }
            case ProgramSegment::Data: {
                const int gp_idx = m_currentProcessor->implementsISA()->gpReg();
                if (gp_idx >= 0) {
                    regs.addInitializationMemory(gp_idx << ceillog2(m_currentProcessor->implementsISA()->bytes()),
                                                 ptrValueBytes.data(), m_currentProcessor->implementsISA()->bytes());
                }
                break;
            }
            default:
                break;
        }

        if (seg.first == ProgramSegment::Stack) {
        }
    }

    // Processor loaded. Request for the currently assembled program to be loaded into the processor
    emit reqReloadProgram();
}

int ProcessorHandler::getCurrentProgramSize() const {
    if (m_program)
        return m_program->text.second.size();
    return 0;
}

QString ProcessorHandler::parseInstrAt(const uint32_t addr) const {
    return Parser::getParser()->disassemble(m_currentProcessor->getMemory().readMem(addr), addr);
}

void ProcessorHandler::handleSysCall() {
    const unsigned int arg = m_currentProcessor->getRegister(10);
    switch (arg) {
        case SysCall::None:
            return;
        case SysCall::PrintStr: {
            QByteArray string;
            char byte;
            unsigned int address = m_currentProcessor->getRegister(11);
            do {
                byte = static_cast<char>(m_currentProcessor->getMemory().readMem(address++) & 0xFF);
                string.append(byte);
            } while (byte != '\0');
            emit print(QString::fromUtf8(string));
            return;
        }
        case SysCall::PrintInt: {
            emit print(QString::number(static_cast<int>(m_currentProcessor->getRegister(11))));
            return;
        }
        case SysCall::PrintChar: {
            QString val = QChar(m_currentProcessor->getRegister(11));
            emit print(val);
            break;
        }
        case SysCall::Exit: {
            m_currentProcessor->finalize();
            return;
        }
        default: {
            QMessageBox::warning(nullptr, "Error",
                                 "Unknown system call argument in register a0: " + QString::number(arg));
            return;
        }
    }
}

void ProcessorHandler::processorFinished() {
    emit exit();
}

void ProcessorHandler::checkValidExecutionRange() const {
    const auto pc = m_currentProcessor->nextPcForStage(0);
    if (!(m_validExecutionRange.first <= pc && pc < m_validExecutionRange.second)) {
        m_currentProcessor->finalize();
    }
}

void ProcessorHandler::setRegisterValue(const unsigned idx, uint32_t value) {
    m_currentProcessor->setRegister(idx, value);
}

uint32_t ProcessorHandler::getRegisterValue(const unsigned idx) const {
    return m_currentProcessor->getRegister(idx);
}
}  // namespace Ripes
