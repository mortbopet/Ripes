#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <memory>

#include "VSRTL/graphics/gallantsignalwrapper.h"
#include "assembler/assembler.h"
#include "assembler/program.h"
#include "processorregistry.h"
#include "processors/interface/ripesprocessor.h"
#include "syscall/ripes_syscall.h"

#include "VSRTL/graphics/vsrtl_widget.h"

namespace Ripes {

/**
 * @brief The ProcessorHandler class
 * Manages construction and destruction of a VSRTL processor design, when selecting between processors.
 * Manages all interaction and control of the current processor.
 */
class ProcessorHandler : public QObject {
    Q_OBJECT

public:
    /// Returns a pointer to the ProcessorHandler singleton.
    static ProcessorHandler* get() {
        static auto* handler = new ProcessorHandler;
        return handler;
    }

    /// Returns a non-const pointer to the currently instantiated processor.
    static RipesProcessor* getProcessorNonConst() { return get()->_getProcessor(); }

    /// Returns a pointer to the currently instantiated processor.
    static const RipesProcessor* getProcessor() { return get()->_getProcessor(); }

    /// Return a pointer to the currently instantiated assembler.
    static const std::shared_ptr<Assembler::AssemblerBase> getAssembler() { return get()->_getAssembler(); }

    /// Returns the ID of the currently instantiated processor.
    static const ProcessorID& getID() { return get()->_getID(); }

    /// Returns a pointer to the currently instantiated program.
    static std::shared_ptr<const Program> getProgram() { return get()->_getProgram(); }

    /// Returns a pointer to the currently instantiated ISA.
    static const ISAInfoBase* currentISA() { return get()->_currentISA(); }

    /// Returns a reference to the system call manager.
    static const SyscallManager& getSyscallManager() { return get()->_getSyscallManager(); }

    /// Sets the program p as the currently instantiated program.
    static void loadProgram(const std::shared_ptr<Program>& p) { get()->_loadProgram(p); }

    /// Returns true if the current processor is a VSRTL-based processor. This may be used to enable VSRTL-specific
    /// functionality, such as processor drawing.
    static bool isVSRTLProcessor();

    /**
     * @brief loadProcessorToWidget
     * Loads the current processor to the @param VSRTLWidget. Required given that ProcessorHandler::getProcessor returns
     * a const ptr.
     */
    static void loadProcessorToWidget(vsrtl::VSRTLWidget* widget, bool doPlaceAndRoute = false) {
        get()->_loadProcessorToWidget(widget, doPlaceAndRoute);
    }

    /**
     * @brief selectProcessor
     * Constructs the processor identified by @param id, and performs all necessary initialization through the
     * RipesProcessor interface.
     */
    static void selectProcessor(const ProcessorID& id, const QStringList& extensions = {},
                                RegisterInitialization setup = RegisterInitialization()) {
        get()->_selectProcessor(id, extensions, setup);
    }

    /**
     * @brief isExecutableAddress
     * @returns whether @param address is within the executable section of the currently loaded program.
     */
    static bool isExecutableAddress(AInt address) { return get()->_isExecutableAddress(address); }

    /**
     * @brief getCurrentProgramSize
     * @return size (in bytes) of the currently loaded .text segment
     */
    static int getCurrentProgramSize() { return get()->_getCurrentProgramSize(); }

    /**
     * @brief getEntryPoint
     * @return address of the entry point of the currently loaded program
     */
    static AInt getTextStart() { return get()->_getTextStart(); }

    /**
     * @brief disassembleInstr
     * @return disassembled representation of the instruction at @param addr in the current program
     */
    static QString disassembleInstr(const AInt address) { return get()->_disassembleInstr(address); }

    /**
     * @brief getMemory
     * returns const-wrapped references to the current process memory
     */
    static vsrtl::core::AddressSpaceMM& getMemory() { return get()->_getMemory(); }

    /**
     * @brief setRegisterValue
     * Set the value of register @param idx to @param value.
     */
    static void setRegisterValue(RegisterFileType rfid, const unsigned idx, VInt value) {
        get()->_setRegisterValue(rfid, idx, value);
    }
    /**
     * @brief writeMem
     * writes @p value from the given @p address start, and up to @p size bytes of @p value into the
     * memory of the simulator
     */
    static void writeMem(AInt address, VInt value, int size = sizeof(VInt)) { get()->_writeMem(address, value, size); }

    /**
     * @brief getRegisterValue
     * @returns value of register @param idx
     */
    static VInt getRegisterValue(RegisterFileType rfid, const unsigned idx) {
        return get()->_getRegisterValue(rfid, idx);
    }

    /// Returns true if the processor is currently at a breakpoint. This is done through comparing the
    /// breakpoint-triggering stages of the current processor, fetching the PC of those stages, and comparing them
    /// against the current set of breakpoint addresses.
    static bool checkBreakpoint() { return get()->_checkBreakpoint(); }

    /// Set/unset the provided address as a breakpoint.
    static void setBreakpoint(const AInt address, bool enabled) { get()->_setBreakpoint(address, enabled); }

    /// Toggles a breakpoint at the provided address.
    static void toggleBreakpoint(const AInt address) { get()->_toggleBreakpoint(address); }

    /// Returns true if there exists a breakpoint at the requested address.
    static bool hasBreakpoint(const AInt address) { return get()->_hasBreakpoint(address); }

    /// Removes all currently set breakpoints.
    static void clearBreakpoints() { get()->_clearBreakpoints(); }

    /// Trigger a processor finished check. This inspect the current processor run state, and if finished, emit a finish
    /// signal.
    static void checkProcessorFinished() { get()->_checkProcessorFinished(); }

    /// Returns true if the simulator is currently in "run" mode.
    static bool isRunning() { return get()->_isRunning(); }

    /**
     * @brief run
     * Asynchronously runs the current processor. During this, the processor will not be emitting signals for updating
     * its graphical representation. Will break upon hitting a breakpoint, going out of bounds wrt. the allowed
     * execution area or if the stop flag has been set through stop().
     */
    static void run() { get()->_run(); }

    static void clock() { get()->_clock(); }

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
    void programChanged();
    void processorReset();
    void processorReversed();
    // Only connect to this if not updating gui!´ i.e., for logging statistics per cycle. Remember to use
    // Qt::DirectConnection for the slot to be executed directly, instead of concurrently in the event loop.
    void processorClocked();
    void processorClockedNonRun();  // Only emitted when _not_ running; i.e., for GUI updating
    void procStateChangedNonRun();  // processorReset | processorReversed | processorClockedNonRun

private slots:
    /**
     * @brief syscallTrap
     * Connects to the processors system call request interface. Will concurrently run the systemcall manager to handle
     * the requested functionality, and return once the system call was handled.
     */
    void syscallTrap();

private:
    /// Private implementations of the ProcessorHandler singleton functions. For documentation, refer to their static
    /// counterparts above.

    void _loadProgram(const std::shared_ptr<Program>& p);
    RipesProcessor* _getProcessor() { return m_currentProcessor.get(); }
    const RipesProcessor* _getProcessor() const { return m_currentProcessor.get(); }
    const std::shared_ptr<Assembler::AssemblerBase> _getAssembler() { return m_currentAssembler; }
    const ProcessorID& _getID() const { return m_currentID; }
    std::shared_ptr<const Program> _getProgram() const { return m_program; }
    const ISAInfoBase* _currentISA() const { return m_currentProcessor->implementsISA(); }
    const SyscallManager& _getSyscallManager() const { return *m_syscallManager; }
    void _loadProcessorToWidget(vsrtl::VSRTLWidget* widget, bool doPlaceAndRoute = false);
    void _selectProcessor(const ProcessorID& id, const QStringList& extensions = {},
                          RegisterInitialization setup = RegisterInitialization());
    bool _isExecutableAddress(AInt address) const;
    int _getCurrentProgramSize() const;
    AInt _getTextStart() const;
    QString _disassembleInstr(const AInt address) const;
    vsrtl::core::AddressSpaceMM& _getMemory();
    const vsrtl::core::AddressSpace& _getRegisters() const;
    void _setRegisterValue(RegisterFileType rfid, const unsigned idx, VInt value);
    void _writeMem(AInt address, VInt value, int size = sizeof(VInt));
    VInt _getRegisterValue(RegisterFileType rfid, const unsigned idx) const;
    bool _checkBreakpoint();
    void _setBreakpoint(const AInt address, bool enabled);
    void _toggleBreakpoint(const AInt address);
    bool _hasBreakpoint(const AInt address) const;
    void _clearBreakpoints();
    void _checkProcessorFinished();
    bool _isRunning();
    void _run();
    void _clock();
    void _reset();
    void _stopRun();
    void _triggerProcStateChangeTimer();

    void createAssemblerForCurrentISA();
    void setStopRunFlag();
    ProcessorHandler();

    // Flag used during construction to avoid calling ProcessorHandler::get() to retrieve the singleton while it is
    // being constructed.
    bool m_constructing = false;
    ProcessorID m_currentID;
    RegisterInitialization m_currentRegInits;
    std::unique_ptr<RipesProcessor> m_currentProcessor;
    std::unique_ptr<SyscallManager> m_syscallManager;
    std::shared_ptr<Assembler::AssemblerBase> m_currentAssembler;

    /**
     * @brief m_vsrtlWidget
     * The VSRTL Widget associated which the processor models will be loaded to
     */
    vsrtl::VSRTLWidget* m_vsrtlWidget = nullptr;

    std::set<AInt> m_breakpoints;
    std::shared_ptr<Program> m_program;

    QFutureWatcher<void> m_runWatcher;
    bool m_stopRunningFlag = false;
    bool m_clockFinished = true;

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
    std::vector<std::unique_ptr<vsrtl::GallantSignalWrapperBase>> m_signalWrappers;
};
}  // namespace Ripes
