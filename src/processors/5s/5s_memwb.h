#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../ss/riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class MEMWB : public Component {
public:
    MEMWB(std::string name, SimComponent* parent) : Component(name, parent) {
        CONNECT_REGISTERED_INPUT(pc);
        CONNECT_REGISTERED_INPUT(pc4);
        CONNECT_REGISTERED_INPUT(alures);
        CONNECT_REGISTERED_INPUT(mem_read);

        CONNECT_REGISTERED_INPUT(reg_wr_src_ctrl);
        CONNECT_REGISTERED_INPUT(wr_reg_idx);
        CONNECT_REGISTERED_INPUT(reg_do_write);

        CONNECT_REGISTERED_INPUT(valid);
    }

    // Data
    REGISTERED_INPUT(pc, RV_REG_WIDTH);
    REGISTERED_INPUT(pc4, RV_REG_WIDTH);
    REGISTERED_INPUT(alures, RV_REG_WIDTH);
    REGISTERED_INPUT(mem_read, RV_REG_WIDTH);

    // Control
    REGISTERED_INPUT(reg_wr_src_ctrl, RegWrSrc::width());
    REGISTERED_INPUT(wr_reg_idx, RV_REGS_BITS);
    REGISTERED_INPUT(reg_do_write, 1);

    // Valid signal. False when the register bank has been cleared. May be used by UI to determine whether the NOP in
    // the stage is a user-inserted nop or the result of some pipeline action.
    REGISTERED_INPUT(valid, 1);
};

}  // namespace core
}  // namespace vsrtl
