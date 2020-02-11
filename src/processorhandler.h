#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>

#include "processorregistry.h"
#include "program.h"

#include "vsrtl_widget.h"

namespace Ripes {

/**
 * @brief The ProcessorHandler class
 * Manages construction and destruction of a VSRTL processor design, when selecting between processors.
 * Manages all interaction and control of the current processor.
 */
class ProcessorHandler : public QObject {
    Q_OBJECT

public:
    static ProcessorHandler* get() {
        static auto* handler = new ProcessorHandler;
        return handler;
    }

    vsrtl::core::RipesProcessor* getProcessorNonConst() { return m_currentProcessor.get(); }
    const vsrtl::core::RipesProcessor* getProcessor() { return m_currentProcessor.get(); }
    const ProcessorID& getID() const { return m_currentID; }
    const Program* getProgram() const { return m_program; }
    const ISAInfoBase* currentISA() const { return m_currentProcessor->implementsISA(); }

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
    void selectProcessor(const ProcessorID& id, RegisterInitialization setup = RegisterInitialization());

    /**
     * @brief checkValidExecutionRange
     * Checks whether the processor, given continued clocking, that it will execute within the currently validated
     * execution range. If the processor in the next cycle will start to fetch instructions outside of the validated
     * range, the processor is instead requested to start finalizing.
     */
    void checkValidExecutionRange() const;

    /**
     * @brief isExecutableAddress
     * @returns whether @param address is within the executable section of the currently loaded program.
     */
    bool isExecutableAddress(uint32_t address) const;

    /**
     * @brief getCurrentProgramSize
     * @return size (in bytes) of the currently loaded .text segment
     */
    int getCurrentProgramSize() const;

    /**
     * @brief getEntryPoint
     * @return address of the entry point of the currently loaded program
     */
    unsigned long getTextStart() const;

    /**
     * @brief parseInstrAt
     * @return string representation of the instruction at @param addr
     */
    QString parseInstrAt(const uint32_t address) const;

    /**
     * @brief getMemory & getRegisters
     * returns const-wrapped references to the current process memory elements
     */
    const vsrtl::core::SparseArray& getMemory() const;
    const vsrtl::core::SparseArray& getRegisters() const;

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

    bool checkBreakpoint();
    void setBreakpoint(const uint32_t address, bool enabled);
    void toggleBreakpoint(const uint32_t address);
    bool hasBreakpoint(const uint32_t address) const;
    void clearBreakpoints();
    void checkProcessorFinished();

    /**
     * @brief run
     * Asynchronously runs the current processor. During this, the processor will not be emitting signals for updating
     * its graphical representation. Will break upon hitting a breakpoint, going out of bounds wrt. the allowed
     * execution area or if the stop flag has been set through stop().
     */
    void run();

    /**
     * @brief stop
     * Sets the m_stopRunningFlag, and waits for any currently running asynchronous run execution to finish.
     */
    void stop();

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

    void runFinished();

public slots:
    void loadProgram(const Program* p);

private slots:
    void handleSysCall();

private:
    ProcessorHandler();

    ProcessorID m_currentID = ProcessorID::RV5S;
    std::unique_ptr<vsrtl::core::RipesProcessor> m_currentProcessor;

    std::set<uint32_t> m_breakpoints;
    const Program* m_program = nullptr;

    QFutureWatcher<void> m_runWatcher;
    bool m_stopRunningFlag = false;
};
}  // namespace Ripes
