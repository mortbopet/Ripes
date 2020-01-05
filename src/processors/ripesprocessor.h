#pragma once

#include <QString>

#include "VSRTL/core/vsrtl_design.h"

namespace Ripes {

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

    void reset() override {
        vsrtl::core::Design::reset();
        m_instructionsRetired = 0;
    }

private:
    // Statistics
    unsigned int m_instructionsRetired = 0;
};

}  // namespace Ripes
