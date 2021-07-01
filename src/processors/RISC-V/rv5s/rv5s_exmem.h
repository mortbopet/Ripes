#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../riscv.h"

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_exmem.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class RV5S_EXMEM : public EXMEM<XLEN> {
public:
    RV5S_EXMEM(const std::string& name, SimComponent* parent) : EXMEM<XLEN>(name, parent) {
        // We want stalling info to persist through clearing of the register, so stalled register is always enabled and
        // never cleared.
        CONNECT_REGISTERED_CLEN_INPUT(stalled, 0, 1);
    }

    REGISTERED_CLEN_INPUT(stalled, 1);
};

}  // namespace core
}  // namespace vsrtl
