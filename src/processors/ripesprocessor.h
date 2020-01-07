#pragma once

#include <QString>

#include <map>
#include "Signals/Signal.h"
#include "VSRTL/core/vsrtl_design.h"

namespace Ripes {

enum SysCall { None = 0, PrintInt = 1, PrintChar = 2, PrintStr = 4, Exit = 10 };

enum class SupportedISA { RISCV };

struct StageInfo {
    QString stageName;
    unsigned int pc;
    bool pc_valid;
};

class RipesProcessor : public vsrtl::core::Design {
public:
    RipesProcessor(std::string name) : vsrtl::core::Design(name) {}

    /**
     * @brief implementsISA
     * @return ISA which this processor implements
     */
    virtual SupportedISA implementsISA() const = 0;

    /**
     * @brief stageCount
     * @return number of stages for the processor
     */
    virtual unsigned int stageCount() const = 0;

    /**
     * @brief breakpointBreaksStage
     * If a breakpoint has been set for address A, the processor will be stopped when A has reached the specified stage
     * @return stage to break in
     */
    virtual unsigned int breakpointBreaksStage() const = 0;

    /**
     * @brief pcForStage
     * @param stageIndex
     * @return Program counter currently present in stage @param stageIndex
     */
    virtual unsigned int pcForStage(unsigned int stageIndex) const = 0;

    /**
     * @brief stageInfo
     * @param stageIndex
     * @return Additional info related to the current execution state of stage @param stageIndex
     */
    virtual StageInfo stageInfo(unsigned int stageIndex) const = 0;

    /**
     * @brief getMemory
     * @return pointer to the address space utilized by the implementing processor
     */
    virtual vsrtl::SparseArray& getMemory() = 0;

    /**
     * @brief getRegister
     * @return value currently present in register @p i
     */
    virtual unsigned int getRegister(unsigned i) = 0;

    /**
     * @brief setProgramCounter
     * Sets the program counter of the processor to @param address
     */
    virtual void setProgramCounter(uint32_t address) = 0;

    void reset() override {
        vsrtl::core::Design::reset();
        m_instructionsRetired = 0;
    }

    Gallant::Signal0<> handleSysCall;

private:
    // Statistics
    unsigned int m_instructionsRetired = 0;
};

}  // namespace Ripes
