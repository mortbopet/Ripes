#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>

#include "assembler/assembler.h"
#include "assembler/program.h"
#include "processorregistry.h"
#include "syscall/ripes_syscall.h"

#include "vsrtl_widget.h"

namespace Ripes {

StatusManager(Processor);

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
    const std::shared_ptr<Assembler::AssemblerBase> getAssembler() { return m_currentAssembler; }
    const ProcessorID& getID() const { return m_currentID; }
    std::shared_ptr<const Program> getProgram() const { return m_program; }
    const ISAInfoBase* currentISA() const { return m_currentProcessor->implementsISA(); }
    const SyscallManager& getSyscallManager() const { return *m_syscallManager; }

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
    void selectProcessor(const ProcessorID& id, const QStringList& extensions = {},
                         RegisterInitialization setup = RegisterInitialization());

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
     * @brief disassembleInstr
     * @return disassembled representation of the instruction at @param addr in the current program
     */
    QString disassembleInstr(const uint32_t address) const;

    /**
     * @brief getMemory & getRegisters
     * returns const-wrapped references to the current process memory elements
     */
    const vsrtl::core::SparseArrayMM& getMemory() const;
    const vsrtl::core::SparseArray& getRegisters() const;
    const vsrtl::core::RVMemory<RV_REG_WIDTH, RV_REG_WIDTH>* getDataMemory() const;
    const vsrtl::core::ROM<RV_REG_WIDTH, RV_INSTR_WIDTH>* getInstrMemory() const;

    /**
     * @brief setRegisterValue
     * Set the value of register @param idx to @param value.
     */
    void setRegisterValue(RegisterFileType rfid, const unsigned idx, uint32_t value);

    /**
     * @brief writeMem
     * writes @p value from the given @p address start, and up to @p size bytes of @p value into the
     * memory of the simulator
     */
    void writeMem(uint32_t address, uint32_t value, int size = sizeof(uint32_t));

    /**
     * @brief getRegisterValue
     * @returns value of register @param idx
     */
    uint32_t getRegisterValue(RegisterFileType rfid, const unsigned idx) const;

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
     * @brief stopRun
     * Sets the m_stopRunningFlag, and waits for any currently running asynchronous run execution to finish.
     */
    void stopRun();

signals:

    /**
     * @brief processorChanged
     * Emitted when a new processor has been chosen.
     */
    void processorChanged();

    /**
     * @brief reqProcessorReset
     *  Emitted whenever changes to the internal state of the processor has been made, and a reset of any depending
     * widgets is required
     */
    void reqProcessorReset();

    /**
     * @brief exit
     * end the current simulation, disallowing further clocking of the processor unless the processor is reset.
     */
    void exit();

    /**
     * @brief stopping
     * Processor has been requested to stop
     */
    void stopping();

    /**
     * @brief runStarted/runFinished
     * Signals indiacting whether the processor is being started/stopped in asynchronously running without any GUI
     * updates.
     */
    void runStarted();
    void runFinished();

public slots:
    void loadProgram(std::shared_ptr<Program> p);

private slots:
    /**
     * @brief asyncTrap
     * Connects to the processors system call request interface. Will concurrently run the systemcall manager to handle
     * the requested functionality, and return once the system call was handled.
     */
    void asyncTrap();

private:
    void createAssemblerForCurrentISA();
    void setStopRunFlag();

    ProcessorHandler();

    ProcessorID m_currentID;
    std::unique_ptr<vsrtl::core::RipesProcessor> m_currentProcessor;
    std::unique_ptr<SyscallManager> m_syscallManager;
    std::shared_ptr<Assembler::AssemblerBase> m_currentAssembler;

    /**
     * @brief m_vsrtlWidget
     * The VSRTL Widget associated which the processor models will be loaded to
     */
    vsrtl::VSRTLWidget* m_vsrtlWidget = nullptr;

    std::set<uint32_t> m_breakpoints;
    std::shared_ptr<Program> m_program;

    QFutureWatcher<void> m_runWatcher;
    bool m_stopRunningFlag = false;

    /**
     * @brief m_sem
     * Semaphore handling locking simulator thread execution whilst trapping to the execution environment.
     */
    QSemaphore m_sem;
};
}  // namespace Ripes
