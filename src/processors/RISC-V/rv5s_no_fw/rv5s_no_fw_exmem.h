#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../riscv.h"

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_exmem.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class RV5S_NO_FW_EXMEM : public EXMEM {
public:
    RV5S_NO_FW_EXMEM(std::string name, SimComponent* parent) : EXMEM(name, parent) {
        
        // Need ctrl-Signal to identify if reg2 is register or immediate
        CONNECT_REGISTERED_CLEN_INPUT(alu_op2_ctrl, clear, enable);
        
        // We want stalling info to persist through clearing of the register, so stalled register is always enabled and
        // never cleared.
        CONNECT_REGISTERED_CLEN_INPUT(stalled, 0, 1);
    }

    REGISTERED_CLEN_INPUT(alu_op2_ctrl, AluSrc2::width());

    REGISTERED_CLEN_INPUT(stalled, 1);
};

}  // namespace core
}  // namespace vsrtl
