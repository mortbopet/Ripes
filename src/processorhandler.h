#pragma once

#include <QObject>

#include "processorregistry.h"

/**
 * @brief The ProcessorHandler class
 * Manages construction and destruction of a VSRTL processor design, when selecting between processors.
 * Manages all interaction and control of the current processor.
 */
class ProcessorHandler : public QObject {
    Q_OBJECT
public:
    ProcessorHandler();

    Ripes::RipesProcessor* getProcessor() { return m_currentProcessor.get(); }
    ProcessorID currentID() const { return m_currentProcessorID; }
    void selectProcessor(ProcessorID id);

signals:
    /**
     * @brief reqProcessorReset
     *  Emitted whenever changes to the internal state of the processor has been made, and a reset of any depending
     * widgets is required
     */
    void reqProcessorReset();

    /**
     * @brief reqReloadProgram
     * Emitted whenever the processor has been changed, and we require the currently assembled program to be inserted
     * into the newly loaded processors memory
     */
    void reqReloadProgram();

    /**
     * @brief print
     * Print string to log
     */
    void print(const QString&);

    /**
     * @brief exit
     * end the current simulation, disallowing further clocking of the processor unless the processor is reset.
     */
    void exit();

public slots:
    void loadProgram(const std::map<uint32_t, QByteArray*>& segments);
    void handleSysCall();

private:
    static constexpr ProcessorID defaultProcessor = ProcessorID::RISCV_SS;
    ProcessorID m_currentProcessorID = defaultProcessor;
    std::unique_ptr<Ripes::RipesProcessor> m_currentProcessor;
};
