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
    enum class State { None, Stalled, Flushed };
    unsigned int pc = 0;
    bool stage_valid = false;
    State state;
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

}  // namespace Ripes

namespace vsrtl {
namespace core {
using namespace Ripes;

class RipesProcessor : public Design {
public:
    RipesProcessor(std::string name) : Design(name) {}

    /**
     * @brief registerFiles
     * @return the set of unique indentifiers for the register files exposed by this processor, under inclusion of the
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
     * @return Address will be fetched from instruction memory in the next clock cycle
     */
    virtual unsigned int nextFetchedAddress() const = 0;

    /**
     * @brief stageInfo
     * @param stageIndex
     * @return Additional info related to the current execution state of stage @param stageIndex
     */
    virtual StageInfo stageInfo(unsigned int stageIndex) const = 0;

    /**
     * @brief getMemory
     * @return reference to the address space utilized by the implementing processor
     */
    virtual SparseArray& getMemory() = 0;

    /**
     * @brief getData/InstrMemory
     * @returns a pointer to the component which implements the instruction and data memory interfaces. These types may
     * be implementation specific, and as such should be cast to these types at the callers end.
     */
    virtual const Component* getDataMemory() const = 0;
    virtual const Component* getInstrMemory() const = 0;

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
    virtual SparseArray& getArchRegisters() = 0;

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

    void reset() override {
        Design::reset();
        m_instructionsRetired = 0;
    }

    /**
     * @brief isExecutableAddress
     * Callback registerred by the environment instantiating the processor. The environment shall return whether the @p
     * address is an address which is valid to be executed.
     */
    std::function<bool(uint32_t)> isExecutableAddress;

    /**
     * @brief handleSysCall
     * Signal for passing control to the outside environment whenever a system call must be handled (RISC-V ecall
     * instruction).
     */
    Gallant::Signal0<> handleSysCall;

    /**
     * @brief finalize
     * Called from the outside environment to indicate that the processor should start or stop its finishing sequence.
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
    long long getInstructionsRetired() const { return m_instructionsRetired; }

protected:
    // Statistics
    long long m_instructionsRetired = 0;
};

}  // namespace core
}  // namespace vsrtl
