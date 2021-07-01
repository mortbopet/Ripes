#pragma once

#include "../rv5s/rv5s_idex.h"
#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class RV5S_IIEX_DUAL : public RV5S_IDEX<XLEN> {
public:
    RV5S_IIEX_DUAL(const std::string& name, SimComponent* parent) : RV5S_IDEX<XLEN>(name, parent) {
        SimBase::setDescription("Instruction issue/execute stage separating register");
        CONNECT_REGISTERED_CLEN_INPUT(r1_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(r2_data, this->clear, this->enable);

        CONNECT_REGISTERED_CLEN_INPUT(wr_reg_idx_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(reg_do_write_data, this->clear, this->enable);

        CONNECT_REGISTERED_CLEN_INPUT(alu_op2_ctrl_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(alu_ctrl_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(imm_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(reg_wr_src_ctrl_dual, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(reg_wr_src_ctrl_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(pc_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(rd_reg1_idx_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(rd_reg2_idx_data, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(exec_valid, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(data_valid, this->clear, this->enable);
        CONNECT_REGISTERED_CLEN_INPUT(instrsize, this->clear, this->enable);
    }

    REGISTERED_CLEN_INPUT(instrsize, XLEN);

    REGISTERED_CLEN_INPUT(pc_data, XLEN);

    REGISTERED_CLEN_INPUT(r1_data, XLEN);
    REGISTERED_CLEN_INPUT(r2_data, XLEN);

    REGISTERED_CLEN_INPUT(rd_reg1_idx_data, c_RVRegsBits);
    REGISTERED_CLEN_INPUT(rd_reg2_idx_data, c_RVRegsBits);

    REGISTERED_CLEN_INPUT(wr_reg_idx_data, c_RVRegsBits);
    REGISTERED_CLEN_INPUT(reg_do_write_data, 1);
    REGISTERED_CLEN_INPUT(reg_wr_src_ctrl_dual, RegWrSrcDual::width());
    REGISTERED_CLEN_INPUT(reg_wr_src_ctrl_data, RegWrSrcDataDual::width());

    REGISTERED_CLEN_INPUT(imm_data, XLEN);

    REGISTERED_CLEN_INPUT(alu_op2_ctrl_data, AluSrc2::width());
    REGISTERED_CLEN_INPUT(alu_ctrl_data, ALUOp::width());

    REGISTERED_CLEN_INPUT(exec_valid, 1);
    REGISTERED_CLEN_INPUT(data_valid, 1);
};

}  // namespace core
}  // namespace vsrtl
