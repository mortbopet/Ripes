#include "processorhandler.h"

#include "processorregistry.h"

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
    emit reqReloadProgram();
}
