#pragma once

#include <QString>

#include <map>
#include "Signals/Signal.h"
#include "VSRTL/core/vsrtl_design.h"

#include "../../isa/isainfo.h"
#include "../../ripes_types.h"

namespace Ripes {

/**
 * @brief The StageInfo struct
 * Contains information regarding the state of the instruction currently present in a given stage, as well as any
 * additional information which the processor may communicate to the GUI regarding the given stage.
 */
struct StageInfo {
    enum class State { None, Stalled, Flushed, WayHazard, Unused };
    AInt pc = 0;
    bool stage_valid = false;
    State state;
    QString namedState = "";
    bool operator==(const StageInfo& other) const {
        return this->pc == other.pc && this->stage_valid == other.stage_valid && this->state == other.state;
    }
    bool operator!=(const StageInfo& other) const { return !(*this == other); }
};

/**
 * @brief The MemoryAccess struct
 * Address is byte-aligned, and the accessed bytes are [address : address + bytes[
 */
struct MemoryAccess {
    enum Type { None, Read, Write };
    Type type = None;
    AInt address;
    unsigned bytes;
};

/**
 * @brief The RipesProcessor class
 * Interface for all Ripes processors. This interface is intended to be simulator-agnostic, and thus provides an opaque
 * interface for retrieving any relevant information required to display the processor state of execution in Ripes.
 *
 * Integer values are communicated in uin64_t variables. If the implementing processor implements a narrower register
 * width, e.g., 32 bit, then only the lower 32 bits should be considered.
 *
 * @todo: Various parts are still dependent on VSRTL structures. For these structures, there should ideally be shim
 * classes to handle the translation from Ripes to the backend.
 */
class RipesProcessor {
public:
    RipesProcessor() {}
    virtual ~RipesProcessor() {}

    /**
     * @brief postConstruct
     * Called after the processor has been constructed. Implementing processors can use this to start any initialization
     * which must be performed after class construction.
     */
    virtual void postConstruct() {}

    /**
     * @brief The Features struct
     * The set of optional features implemented by this processor
     */
    enum Features { isReversible = 0b1, hasICacheInterface = 0b10, hasDCacheInterface = 0b100 };

    unsigned features() const { return m_features; }

    /**
     * @brief registerFiles
     * @return the set of register file types exposed by this processor, under inclusion of the
     * ISA which the processor has been instantiated with.
     */
    virtual const std::set<RegisterFileType> registerFiles() const = 0;

    /**
     * @brief supportsISA
     * @return ISA alongside all of the supported extensions which this processor implements.
     * The type inheriting RipesProcessor must implement a static function which provides information about the
     * supported ISA of the processor.
     */
    // static const ISAInfoBase* T::supportsISA();

    /**
     * @brief implementsISA
     * @return ISA (+extensions) which the _instantiated_ processor implements.
     */
    virtual const ISAInfoBase* implementsISA() const = 0;

    /**
     * @brief stageCount
     * @return number of stages for the processor
     */
    virtual unsigned int stageCount() const = 0;

    /**
     * @brief getPcForStage
     * @param stageIndex
     * @return Program counter currently present in stage @param stageIndex
     */
    virtual unsigned int getPcForStage(unsigned stageIndex) const = 0;

    /**
     * @brief stageName
     * @return name of stage identified by @param stageIndex
     */
    virtual QString stageName(unsigned stageIndex) const = 0;

    /**
     * @brief nextFetchedAddress
     * @return Address which will be fetched from instruction memory in the next clock cycle
     */
    virtual AInt nextFetchedAddress() const = 0;

    /**
     * @brief stageInfo
     * @param stageIndex
     * @return Additional info related to the state of stage @param stageIndex in the current cycle
     */
    virtual StageInfo stageInfo(unsigned stageIndex) const = 0;

    /**
     * @brief breakpointTriggeringStages
     * @returns the stage indices for which a breakpoint is triggered when the breakpoint PC address enters the stage.
     */
    virtual const std::vector<unsigned> breakpointTriggeringStages() const = 0;

    /**
     * @brief getMemory
     * @return reference to the address space utilized by the implementing processor
     */
    virtual vsrtl::core::AddressSpaceMM& getMemory() = 0;

    /**
     * @brief dataMemAccess/instrMemAccess
     * @returns the state of a current access to the instruction or data memory. If the processor did not access the
     * respective memory, MemoryAccess::type == None.
     */
    virtual MemoryAccess dataMemAccess() const = 0;
    virtual MemoryAccess instrMemAccess() const = 0;

    /**
     * @brief getRegister
     * @param rfid: register file identifier
     * @param i: register index
     * @return value currently present in register @p i
     */
    virtual VInt getRegister(RegisterFileType rfid, unsigned i) const = 0;

    /**
     * @brief setRegister
     * @param rfid: register file identifier
     * @param i: register index
     * Set the value of register @param i to @param v.
     */
    virtual void setRegister(RegisterFileType rfid, unsigned i, VInt v) = 0;

    /**
     * @brief setProgramCounter
     * Sets the program counter of the processor to @param address
     */
    virtual void setProgramCounter(AInt address) = 0;

    /**
     * @brief setPCInitialValue
     * Sets the program counters value upon reset to @param address
     */
    virtual void setPCInitialValue(AInt address) = 0;

    /**
     * @brief reset
     * Resets the processor
     */
    virtual void resetProcessor() = 0;

    /**
     * @brief clock
     * Clocks the processor.
     */
    void clock() {
        if (!finished())
            clockProcessor();
    }

    /**
     * @brief finalize
     * Called from Ripes to indicate that the processor should start or stop its finishing sequence.
     * The finishing sequence is defined as executing all remaining instructions in the pipeline, but not fetching new
     * instructions. Typically, finalize would be called once the PC of the processor starts executing outside of the
     * current .text segment, or the processor has executed an exit system call.
     * Stopping the finalizing will happen if the processor returns from fetching instructions from outside the .text
     * segment to inside the .text segment. This will typically happen when a control-flow instruction is near the end
     * of the .text segment.
     */
    enum FinalizeReason { None = 0b0, exitSyscall = 0b10 };
    virtual void finalize(FinalizeReason finalizeReason) = 0;

    /**
     * @brief finished
     * @returns true if the processor has finished executing.
     */
    virtual bool finished() const = 0;

    /**
     * @brief getInstructionsRetired
     * @returns the number of instructions which has retired (ie. executed and no longer in the pipeline).
     */
    virtual long long getInstructionsRetired() const = 0;
    /**
     * @brief getCycleCount
     * @returns the number of cycles which has been executed.
     */
    virtual long long getCycleCount() const = 0;

    /** ======================= Signals and callbacks ======================= */
    /**
     * @brief clocked, reversed & reset signals
     * These signals must be emitted whenever the processor has finished the given operation.
     * Signals should only be emitted if m_emitsSignals is set.
     */
    Gallant::Signal0<> processorWasClocked;
    Gallant::Signal0<> processorWasReversed;
    Gallant::Signal0<> processorWasReset;

    /**
     * @brief isExecutableAddress
     * Callback that the processor can use to query the Ripes environment. Returns whether the @p
     * address is an address which is valid to be executed.
     */
    std::function<bool(AInt)> isExecutableAddress;

    /**
     * @brief trapHandler
     * Callback for the processor to pass control to the Ripes environment whenever a trap must be handled.
     * No arguments are passed - Ripes will look at the syscall register for the ISA of the processor,
     * alongside the syscall argument registers, and act accordingly.
     */
    std::function<void(void)> trapHandler;

    /** ======================== FEATURE: Reversible ======================== */
    // Enabled by setting m_features.isReversible = true

    /**
     * @brief reverse
     * Reverses the processor, undoing the latest clock cycle
     */
    virtual void reverseProcessor() {}
    /**
     * @brief setMaxReverseCycles
     * @p cycles denotes the maximum number of cycles that the processor is expected to be able to reverse.
     */
    virtual void setMaxReverseCycles(unsigned cycles) { Q_UNUSED(cycles); }

    /** ======================================================================*/

protected:
    /**
     * @brief clock
     * Implementation of processor clocking.
     */
    virtual void clockProcessor() = 0;

    // m_features should be adjusted accordingly during processor construction
    unsigned m_features;
    bool m_emitsSignals = true;
};

}  // namespace Ripes
