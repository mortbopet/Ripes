#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../riscv.h"

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_memwb.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class RV5S_MEMWB : public MEMWB<XLEN> {
public:
    RV5S_MEMWB(const std::string& name, SimComponent* parent) : MEMWB<XLEN>(name, parent) {
        CONNECT_REGISTERED_INPUT(stalled);
    }

    REGISTERED_INPUT(stalled, 1);
};

}  // namespace core
}  // namespace vsrtl
