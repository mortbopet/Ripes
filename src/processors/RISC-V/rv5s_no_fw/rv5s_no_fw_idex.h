#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../riscv.h"

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_idex.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The RV5S_NO_FW_IDEX class
 * A specialization of the default IDEX stage separating register utilized by the rv5s_no_fw processor.
 */
template <unsigned XLEN>
class RV5S_NO_FW_IDEX : public IDEX<XLEN> {
public:
    RV5S_NO_FW_IDEX(const std::string& name, SimComponent* parent) : IDEX<XLEN>(name, parent) {
        CONNECT_REGISTERED_CLEN_INPUT(opcode, this->clear, this->enable);

        // We want stalling info to persist through clearing of the register, so stalled register is always enabled and
        // never cleared.
        CONNECT_REGISTERED_CLEN_INPUT(stalled, 0, 1);
    }

    REGISTERED_CLEN_INPUT(opcode, RVInstr::width());
    REGISTERED_CLEN_INPUT(stalled, 1);
};

}  // namespace core
}  // namespace vsrtl
