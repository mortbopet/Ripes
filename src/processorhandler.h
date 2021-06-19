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

    static vsrtl::core::RipesVSRTLProcessor* getProcessorNonConst() { return get()->_getProcessorNonConst(); }
    static const vsrtl::core::RipesVSRTLProcessor* getProcessor() { return get()->_getProcessor(); }
    static const std::shared_ptr<Assembler::AssemblerBase> getAssembler() { return get()->_getAssembler(); }
    static const ProcessorID& getID() { return get()->_getID(); }
    static std::shared_ptr<const Program> getProgram() { return get()->_getProgram(); }
    static const ISAInfoBase* currentISA() { return get()->_currentISA(); }
    static const SyscallManager& getSyscallManager() { return get()->_getSyscallManager(); }

    /**
     * @brief loadProcessorToWidget
     * Loads the current processor to the @param VSRTLWidget. Required given that ProcessorHandler::getProcessor returns
     * a const ptr.
     */
    static void loadProcessorToWidget(vsrtl::VSRTLWidget* widget) { get()->_loadProcessorToWidget(widget); }

    /**
     * @brief selectProcessor
     * Constructs the processor identified by @param id, and performs all necessary initialization through the
     * RipesVSRTLProcessor interface.
     */
    static void selectProcessor(const ProcessorID& id, const QStringList& extensions = {},
                                RegisterInitialization setup = RegisterInitialization()) {
        get()->_selectProcessor(id, extensions, setup);
    }

    /**
     * @brief checkValidExecutionRange
     * Checks whether the processor, given continued clocking, that it will execute within the currently validated
     * execution range. If the processor in the next cycle will start to fetch instructions outside of the validated
     * range, the processor is instead requested to start finalizing.
     */
    static void checkValidExecutionRange() { get()->_checkValidExecutionRange(); }

    /**
     * @brief isExecutableAddress
     * @returns whether @param address is within the executable section of the currently loaded program.
     */
    static bool isExecutableAddress(uint32_t address) { return get()->_isExecutableAddress(address); }

    /**
     * @brief getCurrentProgramSize
     * @return size (in bytes) of the currently loaded .text segment
     */
    static int getCurrentProgramSize() { return get()->_getCurrentProgramSize(); }

    /**
     * @brief getEntryPoint
     * @return address of the entry point of the currently loaded program
     */
    static unsigned long getTextStart() { return get()->_getTextStart(); }

    /**
     * @brief disassembleInstr
     * @return disassembled representation of the instruction at @param addr in the current program
     */
    static QString disassembleInstr(const uint32_t address) { return get()->_disassembleInstr(address); }

    /**
     * @brief getMemory & getRegisters
     * returns const-wrapped references to the current process memory elements
     */
    static vsrtl::core::AddressSpaceMM& getMemory() { return get()->_getMemory(); }
    static const vsrtl::core::AddressSpace& getRegisters() { return get()->_getRegisters(); }
    static const vsrtl::core::BaseMemory<true>* getDataMemory() { return get()->_getDataMemory(); }
    static const vsrtl::core::BaseMemory<true>* getInstrMemory() { return get()->_getInstrMemory(); }

    /**
     * @brief setRegisterValue
     * Set the value of register @param idx to @param value.
     */
    static void setRegisterValue(RegisterFileType rfid, const unsigned idx, uint32_t value) {
        get()->_setRegisterValue(rfid, idx, value);
    }
    /**
     * @brief writeMem
     * writes @p value from the given @p address start, and up to @p size bytes of @p value into the
     * memory of the simulator
     */
    static void writeMem(uint32_t address, uint32_t value, int size = sizeof(uint32_t)) {
        get()->_writeMem(address, value, size);
    }

    /**
     * @brief getRegisterValue
     * @returns value of register @param idx
     */
    static uint32_t getRegisterValue(RegisterFileType rfid, const unsigned idx) {
        return get()->_getRegisterValue(rfid, idx);
    }

    static bool checkBreakpoint() { return get()->_checkBreakpoint(); }
    static void setBreakpoint(const uint32_t address, bool enabled) { get()->_setBreakpoint(address, enabled); }
    static void toggleBreakpoint(const uint32_t address) { get()->_toggleBreakpoint(address); }
    static bool hasBreakpoint(const uint32_t address) { return get()->_hasBreakpoint(address); }
    static void clearBreakpoints() { get()->_clearBreakpoints(); }
    static void checkProcessorFinished() { get()->_checkProcessorFinished(); }
    static bool isRunning() { return get()->_isRunning(); }

    /**
     * @brief run
     * Asynchronously runs the current processor. During this, the processor will not be emitting signals for updating
     * its graphical representation. Will break upon hitting a breakpoint, going out of bounds wrt. the allowed
     * execution area or if the stop flag has been set through stop().
     */
    static void run() { get()->_run(); }

    /**
     * @brief stopRun
     * Sets the m_stopRunningFlag, and waits for any currently running asynchronous run execution to finish.
     */
    static void stopRun() { get()->_stopRun(); }

signals:

    /**
     * @brief processorChanged
     * Emitted when a new processor has been chosen.
     */
    void processorChanged();

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

    /**
     * @brief Various signals wrapping around the direct VSRTL model emission signals. This is done to avoid relying
     * component to having to reconnect to the VSRTL model whenever the processor changes.
     */
    void processorReset();
    void processorReversed();
    // Only connect to this if not updating gui!´ i.e., for logging statistics per cycle. Remember to use
    // Qt::DirectConnection for the slot to be executed directly, instead of concurrently in the event loop.
    void processorClocked();
    void processorClockedNonRun();  // Only emitted when _not_ running; i.e., for GUI updating
    void procStateChangedNonRun();  // processorReset | processorReversed | processorClockedNonRun

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
    vsrtl::core::RipesVSRTLProcessor* _getProcessorNonConst() { return m_currentProcessor.get(); }
    const vsrtl::core::RipesVSRTLProcessor* _getProcessor() { return m_currentProcessor.get(); }
    const std::shared_ptr<Assembler::AssemblerBase> _getAssembler() { return m_currentAssembler; }
    const ProcessorID& _getID() const { return m_currentID; }
    std::shared_ptr<const Program> _getProgram() const { return m_program; }
    const ISAInfoBase* _currentISA() const { return m_currentProcessor->implementsISA(); }
    const SyscallManager& _getSyscallManager() const { return *m_syscallManager; }
    void _loadProcessorToWidget(vsrtl::VSRTLWidget* widget);
    void _selectProcessor(const ProcessorID& id, const QStringList& extensions = {},
                          RegisterInitialization setup = RegisterInitialization());
    void _checkValidExecutionRange() const;
    bool _isExecutableAddress(uint32_t address) const;
    int _getCurrentProgramSize() const;
    unsigned long _getTextStart() const;
    QString _disassembleInstr(const uint32_t address) const;
    vsrtl::core::AddressSpaceMM& _getMemory();
    const vsrtl::core::AddressSpace& _getRegisters() const;
    const vsrtl::core::BaseMemory<true>* _getDataMemory() const;
    const vsrtl::core::BaseMemory<true>* _getInstrMemory() const;
    void _setRegisterValue(RegisterFileType rfid, const unsigned idx, uint32_t value);
    void _writeMem(uint32_t address, uint32_t value, int size = sizeof(uint32_t));
    uint32_t _getRegisterValue(RegisterFileType rfid, const unsigned idx) const;
    bool _checkBreakpoint();
    void _setBreakpoint(const uint32_t address, bool enabled);
    void _toggleBreakpoint(const uint32_t address);
    bool _hasBreakpoint(const uint32_t address) const;
    void _clearBreakpoints();
    void _checkProcessorFinished();
    bool _isRunning();
    void _run();
    void _stopRun();
    void _triggerProcStateChangeTimer();

    /**
     * @brief Wrapper functions for processor signal emissions. VSRTL's signal/slot library does not accept lambdas,
     * which is why we have to make these explicit member functions.
     */
    void processorWasClockedWrapper();
    void processorResetWrapper();
    void processorReversedWrapper();

    void createAssemblerForCurrentISA();
    void setStopRunFlag();
    ProcessorHandler();

    ProcessorID m_currentID;
    std::unique_ptr<vsrtl::core::RipesVSRTLProcessor> m_currentProcessor;
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
     * @brief To avoid excessive UI updates due to things relying on procStateChangedNonRun, the m_procStateChangeTimer
     * ensures that the signal is only emitted with some max. frequency.
     * New state change signals can be enqueued by atomically setting the m_enqueueStateChangeSignal.
     */
    QTimer m_procStateChangeTimer;
    bool m_enqueueStateChangeSignal;
    std::mutex m_enqueueStateChangeLock;

    /**
     * @brief m_sem
     * Semaphore handling locking simulator thread execution whilst trapping to the execution environment.
     */
    QSemaphore m_sem;
};
}  // namespace Ripes
