#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class HazardUnit : public Component {
public:
    HazardUnit(std::string name, SimComponent* parent) : Component(name, parent) {
        hazardEnable << [=] { return !hasHazard(); };
        hazardClear << [=] { return hasHazard(); };
        stallEcallHandling << [=] { return hasEcallHazard(); };
    }

    INPUTPORT(id_reg1_idx, RV_REGS_BITS);
    INPUTPORT(id_reg2_idx, RV_REGS_BITS);

    INPUTPORT(mem_reg_wr_idx, RV_REGS_BITS);
    INPUTPORT(mem_do_read_en, 1);
    INPUTPORT(mem_reg_do_write, 1);

    INPUTPORT(wb_reg_do_write, 1);

    INPUTPORT_ENUM(opcode, RVInstr);

    // Hazard Enable: Low when stalling (shall be connected to a register 'enable' input port
    OUTPUTPORT(hazardEnable, 1);
    // Hazard clear: High when stalling (shall be connected to a register 'clear' input port.
    OUTPUTPORT(hazardClear, 1);
    // Stall Ecall Handling: High whenever we are about to handle an ecall, but have outstanding writes in the pipeline
    // which must be comitted to the register file before handling the ecall.
    OUTPUTPORT(stallEcallHandling, 1);

private:
    bool hasHazard() { return hasLoadUseHazard() || hasEcallHazard(); }

    bool hasLoadUseHazard() const {
        const unsigned midx = mem_reg_wr_idx.uValue();
        const unsigned idx1 = id_reg1_idx.uValue();
        const unsigned idx2 = id_reg2_idx.uValue();
        const bool mrd = mem_do_read_en.uValue();

        return (midx == idx1 || midx == idx2) && mrd;
    }

    bool hasEcallHazard() const {
        // Check for ECALL hazard. We are implictly dependent on all registers when performing an ECALL operation. As
        // such, all outstanding writes to the register file must be performed before handling the ecall. Hence, the
        // front-end of the pipeline shall be stalled until the remainder of the pipeline has been cleared and there are
        // no more outstanding writes.
        const bool isEcall = opcode.uValue() == RVInstr::ECALL;
        return isEcall && (mem_reg_do_write.uValue() || wb_reg_do_write.uValue());
    }
};
}  // namespace core
}  // namespace vsrtl
