#pragma once

#include "processorregistry.h"

/**
 * @brief The ProcessorHandler class
 * Manages construction and destruction of a VSRTL processor design, when selecting between processors.
 * Manages all interaction and control of the current processor.
 */
class ProcessorHandler {
public:
    ProcessorHandler();

    Ripes::RipesProcessor* getProcessor() { return m_currentProcessor.get(); }
    ProcessorID currentID() const { return m_currentProcessorID; }
    void selectProcessor(ProcessorID id);
    void loadProgramData(const std::map<uint32_t, QByteArray&>& segments);

private:
    static constexpr ProcessorID defaultProcessor = ProcessorID::RISCV_SS;
    ProcessorID m_currentProcessorID = defaultProcessor;
    std::unique_ptr<Ripes::RipesProcessor> m_currentProcessor;
};
