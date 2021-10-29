#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class IDEX : public Component {
public:
    IDEX(const std::string& name, SimComponent* parent) : Component(name, parent) {
        setDescription("Instruction decode/execute stage separating register");
        CONNECT_REGISTERED_CLEN_INPUT(pc4, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(pc, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(r1, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(r2, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(imm, clear, enable);

        CONNECT_REGISTERED_CLEN_INPUT(reg_wr_src_ctrl, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(wr_reg_idx, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(reg_do_write, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(alu_op1_ctrl, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(alu_op2_ctrl, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(mem_do_write, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(mem_do_read, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(alu_ctrl, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(mem_op, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(br_op, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(do_br, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(do_jmp, clear, enable);

        CONNECT_REGISTERED_CLEN_INPUT(valid, clear, enable);
    }

    // Data
    REGISTERED_CLEN_INPUT(pc, XLEN);
    REGISTERED_CLEN_INPUT(pc4, XLEN);
    REGISTERED_CLEN_INPUT(r1, XLEN);
    REGISTERED_CLEN_INPUT(r2, XLEN);
    REGISTERED_CLEN_INPUT(imm, XLEN);

    // Control
    REGISTERED_CLEN_INPUT(reg_wr_src_ctrl, RegWrSrc::width());
    REGISTERED_CLEN_INPUT(wr_reg_idx, c_RVRegsBits);
    REGISTERED_CLEN_INPUT(reg_do_write, 1);
    REGISTERED_CLEN_INPUT(alu_op1_ctrl, AluSrc1::width());
    REGISTERED_CLEN_INPUT(alu_op2_ctrl, AluSrc2::width());
    REGISTERED_CLEN_INPUT(alu_ctrl, ALUOp::width());
    REGISTERED_CLEN_INPUT(mem_do_write, 1);
    REGISTERED_CLEN_INPUT(mem_do_read, 1);
    REGISTERED_CLEN_INPUT(mem_op, MemOp::width());
    REGISTERED_CLEN_INPUT(br_op, CompOp::width());
    REGISTERED_CLEN_INPUT(do_br, 1);
    REGISTERED_CLEN_INPUT(do_jmp, 1);

    // Register bank controls
    INPUTPORT(enable, 1);
    INPUTPORT(clear, 1);

    // Valid signal. False when the register bank has been cleared. May be used by UI to determine whether the NOP in
    // the stage is a user-inserted nop or the result of some pipeline action.
    REGISTERED_CLEN_INPUT(valid, 1);
};

}  // namespace core
}  // namespace vsrtl
