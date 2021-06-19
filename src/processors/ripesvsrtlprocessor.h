#pragma once

/**
 * Interface for all VSRTL-based Ripes processors
 */

#include "VSRTL/core/vsrtl_design.h"
#include "ripesprocessor.h"

namespace Ripes {

class RipesVSRTLProcessor : public RipesProcessor, public vsrtl::core::Design {
public:
    RipesVSRTLProcessor(std::string name) : Design(name) {}

    virtual void reset() override {
        m_instructionsRetired = 0;
        Design::reset();
    }

    long long getInstructionsRetired() const override { return m_instructionsRetired; }

protected:
    // Statistics
    long long m_instructionsRetired = 0;
};

}  // namespace Ripes
