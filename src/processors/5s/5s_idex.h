#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../ss/riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class IDEX : public Component {
public:
    IDEX(std::string name, SimComponent* parent) : Component(name, parent) {
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
        CONNECT_REGISTERED_CLEN_INPUT(alu_ctrl, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(mem_op, clear, enable);
    }

    // Data
    REGISTERED_CLEN_INPUT(pc, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(pc4, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(r1, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(r2, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(imm, RV_REG_WIDTH);

    // Control
    REGISTERED_CLEN_INPUT(reg_wr_src_ctrl, RegWrSrc::width());
    REGISTERED_CLEN_INPUT(wr_reg_idx, RV_REGS_BITS);
    REGISTERED_CLEN_INPUT(reg_do_write, 1);
    REGISTERED_CLEN_INPUT(alu_op1_ctrl, AluSrc1::width());
    REGISTERED_CLEN_INPUT(alu_op2_ctrl, AluSrc2::width());
    REGISTERED_CLEN_INPUT(alu_ctrl, ALUOp::width());
    REGISTERED_CLEN_INPUT(mem_do_write, 1);
    REGISTERED_CLEN_INPUT(mem_op, MemOp::width());

    // Register bank controls
    INPUTPORT(enable, 1);
    INPUTPORT(clear, 1);
};

}  // namespace core
}  // namespace vsrtl
