#pragma once

/**
 * Interface for all VSRTL-based Ripes processors
 */

#include "VSRTL/core/vsrtl_design.h"
#include "interface/ripesprocessor.h"

namespace Ripes {

class RipesVSRTLProcessor : public RipesProcessor, public vsrtl::core::Design {
public:
    RipesVSRTLProcessor(std::string name) : Design(name) {
        // VSRTL provides reversible simulation
        m_features = {Features::isReversible | Features::hasDCacheInterface | Features::hasICacheInterface};

        // Shim signal emissions from VSRTL to RipesProcessor
        designWasClocked.Connect(&processorWasClocked, &Gallant::Signal0<>::Emit);
        designWasReversed.Connect(&processorWasReversed, &Gallant::Signal0<>::Emit);
        designWasReset.Connect(&processorWasReset, &Gallant::Signal0<>::Emit);
    }

    virtual void resetProcessor() override {
        m_instructionsRetired = 0;
        reset();
    }

    virtual void clockProcessor() override { clock(); }
    virtual void reverseProcessor() override { reverse(); }

    long long getInstructionsRetired() const override { return m_instructionsRetired; }
    long long getCycleCount() const override { return m_cycleCount; }
    void setMaxReverseCycles(unsigned cycles) override { setReverseStackSize(cycles); }

    void postConstruct() override {
        /**
         * VSRTL designs must call verifyAndInitialize after being constructed.
         */
        verifyAndInitialize();
    }

protected:
    // m_instructionsRetired should be modified by the processor when it retires (or "un-retires", while reversing) an
    // instruction
    long long m_instructionsRetired = 0;
};

}  // namespace Ripes
