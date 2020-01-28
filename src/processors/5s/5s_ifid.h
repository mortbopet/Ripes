#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../ss/riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class IFID : public Component {
public:
    IFID(std::string name, SimComponent* parent) : Component(name, parent) {
        CONNECT_REGISTERED_CLEN_INPUT(pc4, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(pc, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(instr, clear, enable);
    }

    REGISTERED_CLEN_INPUT(pc4, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(instr, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(pc, RV_REG_WIDTH);

    // Register controls
    INPUTPORT(enable, 1);
    INPUTPORT(clear, 1);
};

}  // namespace core
}  // namespace vsrtl
