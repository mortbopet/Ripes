#pragma once

#include <QObject>

#include "processorregistry.h"
#include "program.h"

/**
 * @brief The ProcessorHandler class
 * Manages construction and destruction of a VSRTL processor design, when selecting between processors.
 * Manages all interaction and control of the current processor.
 */
class ProcessorHandler : public QObject {
    Q_OBJECT
public:
    ProcessorHandler();

    /// @todo: this should return a const ptr - ONLY the processorhandler is allowed to modify the state of the
    /// processor
    Ripes::RipesProcessor* getProcessor() { return m_currentProcessor.get(); }
    ProcessorID currentID() const { return m_currentProcessorID; }

    /**
     * @brief selectProcessor
     * Constructs the processor identified by @param id, and performs all necessary initialization through the
     * RipesProcessor interface.
     */
    void selectProcessor(const ProcessorID id);

    /**
     * @brief checkValidExecutionRange
     * Checks whether the processor, given continued clocking, that it will execute within the currently validated
     * execution range. If the processor in the next cycle will start to fetch instructions outside of the validated
     * range, the processor is instead requested to start finalizing.
     */
    void checkValidExecutionRange() const;

    /**
     * @brief getCurrentProgramSize
     * @return size (in bytes) of the currently loaded .text segment
     */
    int getCurrentProgramSize() const;

    /**
     * @brief parseInstrAt
     * @return string representation of the instruction at @param addr
     */
    QString parseInstrAt(const uint32_t address) const;

    /**
     * @brief setRegisterValue
     * Set the value of register @param idx to @param value.
     */
    void setRegisterValue(const unsigned idx, uint32_t value);

    /**
     * @brief getRegisterValue
     * @returns value of register @param idx
     */
    uint32_t getRegisterValue(const unsigned idx) const;

    void setBreakpoint(const uint32_t address, bool enabled);
    bool hasBreakpoint(const uint32_t address) const;

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
    void loadProgram(const Program& p);

private slots:
    void handleSysCall();
    void processorFinished();

private:
    static constexpr ProcessorID defaultProcessor = ProcessorID::RISCV_SS;
    ProcessorID m_currentProcessorID = defaultProcessor;
    std::unique_ptr<Ripes::RipesProcessor> m_currentProcessor;

    /**
     * @brief m_validExecutionRange
     * Address range which the processor is allowed to fetch instructions from. Currently, only a single range is
     * supported which shall encapsulate the .text segment. If, for instance JIT compilation and execution is required
     * in the future, this range should be expanded to also include the .data or other relevant program sections.
     */
    std::pair<uint32_t, uint32_t> m_validExecutionRange;
    std::set<uint32_t> m_breakpoints;
    Program m_program;
};
