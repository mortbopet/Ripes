#pragma once

#include <QString>

#include <map>
#include "Signals/Signal.h"
#include "VSRTL/core/vsrtl_design.h"

#include "../isa/isainfo.h"

namespace Ripes {

/**
 * @brief The StageInfo struct
 * Contains information regarding the state of the instruction currently present in a given stage, as well as any
 * additional information which the processor may communicate to the GUI regarding the given stage.
 */
struct StageInfo {
    enum class State { None, Stalled, Flushed, WayHazard, Unused };
    vsrtl::VSRTL_VT_U pc = 0;
    bool stage_valid = false;
    State state;
    bool operator==(const StageInfo& other) const {
        return this->pc == other.pc && this->stage_valid == other.stage_valid && this->state == other.state;
    }
    bool operator!=(const StageInfo& other) const { return !(*this == other); }
};

/**
 * @brief The FinalizeReason struct
 * Transmitted to the processor to indicate the reason for why the finalization sequence has been initialized.
 */
struct FinalizeReason {
    bool exitedExecutableRegion = false;
    bool exitSyscall = false;
    bool any() const { return exitedExecutableRegion || exitSyscall; }
};

/**
 * @brief The RipesProcessor class
 * Interface for all Ripes processors. This interface is intended to be simulator-agnostic, and thus provides an opaque
 * interface for retrieving any relevant information required to display the processor state of execution in Ripes.
 *
 * @todo: Various parts are still dependent on VSRTL structures. For these structures, there should ideally be shim
 * classes to handle the translation from Ripes to the backend.
 */
class RipesProcessor {
public:
    RipesProcessor() {}

    /**
     * @brief registerFiles
     * @return the set of register file types exposed by this processor, under inclusion of the
     * ISA which the processor has been instantiated with.
     */
    virtual const std::set<RegisterFileType> registerFiles() const = 0;

    /**
     * @brief supportsISA
     * @return ISA alongside all of the supported extensions which this processor implements.
     */
    virtual const ISAInfoBase* supportsISA() const = 0;

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
    virtual unsigned int getPcForStage(unsigned int stageIndex) const = 0;

    /**
     * @brief stageName
     * @return name of stage identified by @param stageIndex
     */
    virtual QString stageName(unsigned int stageIndex) const = 0;

    /**
     * @brief nextFetchedAddress
     * @return Address which will be fetched from instruction memory in the next clock cycle
     */
    virtual unsigned int nextFetchedAddress() const = 0;

    /**
     * @brief stageInfo
     * @param stageIndex
     * @return Additional info related to the state of stage @param stageIndex in the current cycle
     */
    virtual StageInfo stageInfo(unsigned int stageIndex) const = 0;

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
     * @brief getData/InstrMemory
     * @returns a pointer to the component which implements the instruction and data memory interfaces. These types may
     * be implementation specific, and as such should be cast to these types at the callers end.
     */
    virtual const vsrtl::core::BaseMemory<true>* getDataMemory() const = 0;
    virtual const vsrtl::core::BaseMemory<true>* getInstrMemory() const = 0;

    /**
     * @brief getRegister
     * @param rfid: register file identifier
     * @param i: register index
     * @return value currently present in register @p i
     */
    virtual unsigned int getRegister(RegisterFileType rfid, unsigned i) const = 0;

    /**
     * @brief getArchRegisters
     * @return reference to the register address space utilized by the implementing processor
     */
    virtual vsrtl::core::AddressSpace& getArchRegisters() = 0;

    /**
     * @brief setRegister
     * @param rfid: register file identifier
     * @param i: register index
     * Set the value of register @param i to @param v.
     */
    virtual void setRegister(RegisterFileType rfid, unsigned i, uint32_t v) = 0;

    /**
     * @brief setProgramCounter
     * Sets the program counter of the processor to @param address
     */
    virtual void setProgramCounter(uint32_t address) = 0;

    /**
     * @brief setPCInitialValue
     * Sets the program counters value upon reset to @param address
     */
    virtual void setPCInitialValue(uint32_t address) = 0;

    /**
     * @brief reset
     * Resets the processor
     */
    virtual void reset() = 0;

    /**
     * @brief isExecutableAddress
     * Callback that the processor can use to query the Ripes environment. Returns whether the @p
     * address is an address which is valid to be executed.
     */
    std::function<bool(uint32_t)> isExecutableAddress;

    /**
     * @brief handleSysCall
     * Signal that the processor can use to pass control to the outside environment whenever a system call must be
     * handled (RISC-V ecall instruction).
     */
    Gallant::Signal0<> handleSysCall;

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
    virtual void finalize(const FinalizeReason&) = 0;

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
};

}  // namespace Ripes
