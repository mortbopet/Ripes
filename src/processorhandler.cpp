#include "processorhandler.h"

#include "parser.h"
#include "processorregistry.h"
#include "program.h"

#include <QMessageBox>

ProcessorHandler::ProcessorHandler() {
    // Contruct the default processor
    selectProcessor(m_currentProcessorID);
}

void ProcessorHandler::loadProgram(const Program& p) {
    auto& mem = m_currentProcessor->getMemory();

    m_program = p;
    mem.clearInitializationMemories();
    mem.addInitializationMemory(p.text.first, p.text.second->data(), p.text.second->length());
    for (const auto& seg : p.others) {
        mem.addInitializationMemory(seg.first, seg.second->data(), seg.second->length());
    }

    // Set the valid execution range to be contained within the .text segment.
    m_validExecutionRange = {p.text.first, p.text.first + p.text.second->length()};

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

void ProcessorHandler::setBreakpoint(const uint32_t address, bool enabled) {
    if (enabled) {
        m_breakpoints.insert(address);
    } else if (m_breakpoints.count(address)) {
        m_breakpoints.erase(address);
    }
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

void ProcessorHandler::selectProcessor(const ProcessorID id) {
    m_currentProcessorID = id;
    m_currentProcessor = ProcessorRegistry::constructProcessor(m_currentProcessorID);
    m_currentProcessor->handleSysCall.Connect(this, &ProcessorHandler::handleSysCall);
    m_currentProcessor->finished.Connect(this, &ProcessorHandler::processorFinished);
    emit reqReloadProgram();
}

int ProcessorHandler::getCurrentProgramSize() const {
    return m_program.text.second->size();
}

QString ProcessorHandler::parseInstrAt(const uint32_t addr) const {
    return Parser::getParser()->genStringRepr(m_currentProcessor->getMemory().readMem(addr), addr);
}

void ProcessorHandler::handleSysCall() {
    const unsigned int arg = m_currentProcessor->getRegister(10);
    switch (arg) {
        case Ripes::SysCall::None:
            return;
        case Ripes::SysCall::PrintStr: {
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
        case Ripes::SysCall::PrintInt: {
            emit print(QString::number(static_cast<int>(m_currentProcessor->getRegister(11))));
            return;
        }
        case Ripes::SysCall::PrintChar: {
            QString val = QChar(m_currentProcessor->getRegister(11));
            emit print(val);
            break;
        }
        case Ripes::SysCall::Exit: {
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
