#pragma once

/**
 * Interface for all VSRTL-based Ripes processors
 */

#include "RISC-V/riscv.h"
#include "VSRTL/core/vsrtl_design.h"
#include "interface/ripesprocessor.h"

namespace Ripes {

class RipesVSRTLProcessor : public RipesProcessor, public vsrtl::core::Design {
public:
    RipesVSRTLProcessor(const std::string& name) : Design(name) {
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
    MemoryAccess memToAccessInfo(const vsrtl::core::BaseMemory<true>* memory) const {
        MemoryAccess access;
        switch (memory->opSig()) {
            case MemOp::SB: {
                access.bytes = 1;
                access.type = MemoryAccess::Write;
                break;
            }
            case MemOp::SH: {
                access.bytes = 2;
                access.type = MemoryAccess::Write;
                break;
            }
            case MemOp::SW: {
                access.bytes = 4;
                access.type = MemoryAccess::Write;
                break;
            }
            case MemOp::SD: {
                access.bytes = 8;
                access.type = MemoryAccess::Write;
                break;
            }
            case MemOp::LB:
            case MemOp::LBU: {
                access.bytes = 1;
                access.type = MemoryAccess::Read;
                break;
            }
            case MemOp::LH:
            case MemOp::LHU: {
                access.bytes = 2;
                access.type = MemoryAccess::Read;
                break;
            }
            case MemOp::LW:
            case MemOp::LWU: {
                access.bytes = 4;
                access.type = MemoryAccess::Read;
                break;
            }
            case MemOp::LD: {
                access.bytes = 8;
                access.type = MemoryAccess::Read;
                break;
            }
            case MemOp::NOP:
                access.type = MemoryAccess::None;
                break;
            default:
                Q_ASSERT("Unknown memory operation");
                break;
        }
        access.address = memory->addressSig();
        return access;
    }

    // m_instructionsRetired should be modified by the processor when it retires (or "un-retires", while reversing)
    // an instruction
    long long m_instructionsRetired = 0;
};

}  // namespace Ripes
