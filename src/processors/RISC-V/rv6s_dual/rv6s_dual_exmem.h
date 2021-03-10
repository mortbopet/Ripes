#pragma once

#include "../rv5s/rv5s_exmem.h"
#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class RV5S_EXMEM_DUAL : public RV5S_EXMEM {
public:
    RV5S_EXMEM_DUAL(std::string name, SimComponent* parent) : RV5S_EXMEM(name, parent) {
        CONNECT_REGISTERED_CLEN_INPUT(wr_reg_idx_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(reg_do_write_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(reg_wr_src_ctrl_dual, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(reg_wr_src_ctrl_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(pc_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(alures_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(exec_valid, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(data_valid, clear, enable);
    }

    REGISTERED_CLEN_INPUT(wr_reg_idx_data, RV_REGS_BITS);
    REGISTERED_CLEN_INPUT(reg_do_write_data, 1);
    REGISTERED_CLEN_INPUT(reg_wr_src_ctrl_dual, RegWrSrcDual::width());
    REGISTERED_CLEN_INPUT(reg_wr_src_ctrl_data, RegWrSrcDataDual::width());
    REGISTERED_CLEN_INPUT(pc_data, RV_REG_WIDTH);

    REGISTERED_CLEN_INPUT(alures_data, RV_REG_WIDTH);

    REGISTERED_CLEN_INPUT(exec_valid, 1);
    REGISTERED_CLEN_INPUT(data_valid, 1);
};

}  // namespace core
}  // namespace vsrtl
