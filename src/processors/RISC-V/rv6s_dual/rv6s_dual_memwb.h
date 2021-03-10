#pragma once

#include "../rv5s/rv5s_memwb.h"
#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class RV5S_MEMWB_DUAL : public RV5S_MEMWB {
public:
    RV5S_MEMWB_DUAL(std::string name, SimComponent* parent) : RV5S_MEMWB(name, parent) {
        CONNECT_REGISTERED_INPUT(wr_reg_idx_data);
        CONNECT_REGISTERED_INPUT(reg_do_write_data);
        CONNECT_REGISTERED_INPUT(reg_wr_src_ctrl_dual);
        CONNECT_REGISTERED_INPUT(reg_wr_src_ctrl_data);
        CONNECT_REGISTERED_INPUT(pc_data);
        CONNECT_REGISTERED_INPUT(exec_valid);
        CONNECT_REGISTERED_INPUT(data_valid);
        CONNECT_REGISTERED_INPUT(alures_data);
    }

    REGISTERED_INPUT(wr_reg_idx_data, RV_REGS_BITS);
    REGISTERED_INPUT(reg_do_write_data, 1);
    REGISTERED_INPUT(reg_wr_src_ctrl_dual, RegWrSrcDual::width());
    REGISTERED_INPUT(reg_wr_src_ctrl_data, RegWrSrcDataDual::width());
    REGISTERED_INPUT(pc_data, RV_REG_WIDTH);
    REGISTERED_INPUT(alures_data, RV_REG_WIDTH);

    // Valid signals for each way (not register clearing)
    REGISTERED_INPUT(exec_valid, 1);
    REGISTERED_INPUT(data_valid, 1);
};

}  // namespace core
}  // namespace vsrtl
