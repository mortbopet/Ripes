#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class HazardUnit : public Component {
public:
    HazardUnit(const std::string& name, SimComponent* parent) : Component(name, parent) {
        hazardFEEnable << [=] { return !hasHazard(); };
        hazardIDEXEnable << [=] { return !hasEcallHazard(); };
        hazardEXMEMClear << [=] { return hasEcallHazard(); };
        hazardIDEXClear << [=] { return hasLoadUseHazard(); };
        stallEcallHandling << [=] { return hasEcallHazard(); };
    }

    INPUTPORT(id_reg1_idx, c_RVRegsBits);
    INPUTPORT(id_reg2_idx, c_RVRegsBits);

    INPUTPORT(ex_reg_wr_idx, c_RVRegsBits);
    INPUTPORT(ex_do_mem_read_en, 1);

    INPUTPORT(mem_do_reg_write, 1);

    INPUTPORT(wb_do_reg_write, 1);

    INPUTPORT_ENUM(opcode, RVInstr);

    // Hazard Front End enable: Low when stalling the front end (shall be connected to a register 'enable' input port).
    // The
    OUTPUTPORT(hazardFEEnable, 1);

    // Hazard IDEX enable: Low when stalling due to an ECALL hazard
    OUTPUTPORT(hazardIDEXEnable, 1);

    // EXMEM clear: High when an ECALL hazard is detected
    OUTPUTPORT(hazardEXMEMClear, 1);
    // IDEX clear: High when a load-use hazard is detected
    OUTPUTPORT(hazardIDEXClear, 1);

    // Stall Ecall Handling: High whenever we are about to handle an ecall, but have outstanding writes in the pipeline
    // which must be comitted to the register file before handling the ecall.
    OUTPUTPORT(stallEcallHandling, 1);

private:
    bool hasHazard() { return hasLoadUseHazard() || hasEcallHazard(); }

    bool hasLoadUseHazard() const {
        const unsigned exidx = ex_reg_wr_idx.uValue();
        const unsigned idx1 = id_reg1_idx.uValue();
        const unsigned idx2 = id_reg2_idx.uValue();
        const bool mrd = ex_do_mem_read_en.uValue();

        return (exidx == idx1 || exidx == idx2) && mrd;
    }

    bool hasEcallHazard() const {
        // Check for ECALL hazard. We are implictly dependent on all registers when performing an ECALL operation. As
        // such, all outstanding writes to the register file must be performed before handling the ecall. Hence, the
        // front-end of the pipeline shall be stalled until the remainder of the pipeline has been cleared and there are
        // no more outstanding writes.
        const bool isEcall = opcode.uValue() == RVInstr::ECALL;
        return isEcall && (mem_do_reg_write.uValue() || wb_do_reg_write.uValue());
    }
};
}  // namespace core
}  // namespace vsrtl
