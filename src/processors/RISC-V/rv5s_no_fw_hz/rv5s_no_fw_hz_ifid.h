#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class IFID : public Component {
public:
    IFID(const std::string& name, SimComponent* parent) : Component(name, parent) {
        setDescription("Instruction fetch/Instruction decode stage separating register");
        CONNECT_REGISTERED_CLEN_INPUT(pc4, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(pc, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(instr, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(valid, clear, enable);
    }

    REGISTERED_CLEN_INPUT(pc4, XLEN);
    REGISTERED_CLEN_INPUT(instr, c_RVInstrWidth);
    REGISTERED_CLEN_INPUT(pc, XLEN);

    // Register controls
    INPUTPORT(enable, 1);
    INPUTPORT(clear, 1);

    // Valid signal. False when the register bank has been cleared. May be used by UI to determine whether the NOP in
    // the stage is a user-inserted nop or the result of some pipeline action.
    REGISTERED_CLEN_INPUT(valid, 1);
};

}  // namespace core
}  // namespace vsrtl
