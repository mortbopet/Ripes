#include "processorhandler.h"

#include "processorregistry.h"

#include <QMessageBox>

ProcessorHandler::ProcessorHandler() {
    // Contruct the default processor
    selectProcessor(m_currentProcessorID);
}

void ProcessorHandler::loadProgram(const std::map<uint32_t, QByteArray*>& segments) {
    auto& mem = m_currentProcessor->getMemory();

    mem.clearInitializationMemories();
    for (const auto& seg : segments) {
        mem.addInitializationMemory(seg.first, seg.second->data(), seg.second->length());
    }

    emit reqProcessorReset();
}

void ProcessorHandler::selectProcessor(ProcessorID id) {
    m_currentProcessorID = id;
    m_currentProcessor = ProcessorRegistry::constructProcessor(m_currentProcessorID);
    m_currentProcessor->handleSysCall.Connect(this, &ProcessorHandler::handleSysCall);
    m_currentProcessor->finished.Connect(this, &ProcessorHandler::processorFinished);
    emit reqReloadProgram();
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

void ProcessorHandler::canClockProcessor() const {
    const auto pc = m_currentProcessor->pcForStage(0);
}
