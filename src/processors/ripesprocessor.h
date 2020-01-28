#pragma once

#include <QString>

#include <map>
#include "Signals/Signal.h"
#include "VSRTL/core/vsrtl_design.h"

#include "../isainfo.h"

namespace Ripes {
enum SysCall {
    None = 0,
    PrintInt = 1,
    PrintFloat = 2,
    PrintStr = 4,
    Exit = 10,
    PrintChar = 11,
    PrintIntHex = 34,
    PrintIntBinary = 35,
    PrintIntUnsigned = 36,
    Exit2 = 93
};

struct StageInfo {
    unsigned int pc = 0;
    bool pc_valid = false;
};
}  // namespace Ripes

namespace vsrtl {
namespace core {
using namespace Ripes;

class RipesProcessor : public Design {
public:
    RipesProcessor(std::string name) : Design(name) {}

    /**
     * @brief implementsISA
     * @return ISA which this processor implements
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
     * @brief nextPcForStage
     * @param stageIndex
     * @return Next-state program counter for stage @param stageIndex
     */
    virtual unsigned int nextPcForStage(unsigned int stageIndex) const = 0;

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
     * @brief getRegister
     * @return value currently present in register @p i
     */
    virtual unsigned int getRegister(unsigned i) const = 0;

    /**
     * @brief getRegisters
     * @return reference to the register address space utilized by the implementing processor
     */
    virtual SparseArray& getRegisters() = 0;

    /**
     * @brief setRegister
     * Set the value of register @param i to @param v.
     */
    virtual void setRegister(unsigned i, uint32_t v) = 0;

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
     * @brief handleSysCall
     * Signal for passing control to the outside environment whenever a system call must be handled (RISC-V ecall
     * instruction).
     */
    Gallant::Signal0<> handleSysCall;

    /**
     * @brief finalize
     * Called from the outside environment to indicate that the processor should begin its finishing sequence. The
     * finishing sequence is defined as executing all remaining instructions in the pipeline, but not fetching new
     * instructions. Typically, finalize would be called once the PC of the processor starts executing outside of the
     * current .text segment, or the processor has executed an exit system call.
     */
    virtual void finalize() = 0;

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
