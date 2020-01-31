#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../riscv.h"

#include "../rv5swof/rv5swof_idex.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The RV5S_IDEX class
 * A specialization of the default IDEX stage separating register utilized by the RV5SWOF processor. Storage of register
 * read indices is added, which are required by the forwarding unit.
 */
class RV5S_IDEX : public IDEX {
public:
    RV5S_IDEX(std::string name, SimComponent* parent) : IDEX(name, parent) {
        CONNECT_REGISTERED_CLEN_INPUT(rd_reg1_idx, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(rd_reg2_idx, clear, enable);
    }

    REGISTERED_CLEN_INPUT(rd_reg1_idx, RV_REGS_BITS);
    REGISTERED_CLEN_INPUT(rd_reg2_idx, RV_REGS_BITS);
};

}  // namespace core
}  // namespace vsrtl
