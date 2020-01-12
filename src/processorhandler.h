#pragma once

#include <QObject>

#include "processorregistry.h"
#include "program.h"

#include "vsrtl_widget.h"

/**
 * @brief The ProcessorHandler class
 * Manages construction and destruction of a VSRTL processor design, when selecting between processors.
 * Manages all interaction and control of the current processor.
 */
class ProcessorHandler : public QObject {
    Q_OBJECT
    friend class VSRTLWidget;

public:
    ProcessorHandler();

    const Ripes::RipesProcessor* getProcessor() { return m_currentProcessor.get(); }
    const ProcessorSetup& getSetup() const { return m_currentSetup; }

    /**
     * @brief loadProcessorToWidget
     * Loads the current processor to the @param VSRTLWidget. Required given that ProcessorHandler::getProcessor returns
     * a const ptr.
     */
    void loadProcessorToWidget(vsrtl::VSRTLWidget* widget);

    /**
     * @brief selectProcessor
     * Constructs the processor identified by @param id, and performs all necessary initialization through the
     * RipesProcessor interface.
     */
    void selectProcessor(const ProcessorSetup& id);

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
     * @brief getMemory & getRegisters
     * returns const-wrapped references to the current process memory elements
     */
    const vsrtl::SparseArray& getMemory() const;
    const vsrtl::SparseArray& getRegisters() const;

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

    void checkBreakpoint();
    void setBreakpoint(const uint32_t address, bool enabled);
    void toggleBreakpoint(const uint32_t address);
    bool hasBreakpoint(const uint32_t address) const;
    void clearBreakpoints();

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

    /**
     * @brief hitBreakpoint
     * Emitted whenever the instruction at breakpoint @param address is fetched into the processor
     */
    void hitBreakpoint(uint32_t address);

public slots:
    void loadProgram(const Program& p);

private slots:
    void handleSysCall();
    void processorFinished();

private:
    ProcessorSetup m_currentSetup = ProcessorRegistry::getDescription(ProcessorID::RISCV_SS).defaultSetup;
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
