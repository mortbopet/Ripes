#pragma once

#include "../rv5s/rv5s_memwb.h"
#include "rv6s_dual_common.h"

#include "VSRTL/interface/vsrtl_interface.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class RV5S_MEMWB_DUAL : public RV5S_MEMWB<XLEN> {
public:
    RV5S_MEMWB_DUAL(const std::string& name, SimComponent* parent) : RV5S_MEMWB<XLEN>(name, parent) {
        CONNECT_REGISTERED_INPUT(wr_reg_idx_data);
        CONNECT_REGISTERED_INPUT(reg_do_write_data);
        CONNECT_REGISTERED_INPUT(reg_wr_src_ctrl_dual);
        CONNECT_REGISTERED_INPUT(reg_wr_src_ctrl_data);
        CONNECT_REGISTERED_INPUT(pc_data);
        CONNECT_REGISTERED_INPUT(exec_valid);
        CONNECT_REGISTERED_INPUT(data_valid);
        CONNECT_REGISTERED_INPUT(alures_data);
    }

    REGISTERED_INPUT(wr_reg_idx_data, c_RVRegsBits);
    REGISTERED_INPUT(reg_do_write_data, 1);
    REGISTERED_INPUT(reg_wr_src_ctrl_dual, RegWrSrcDual::width());
    REGISTERED_INPUT(reg_wr_src_ctrl_data, RegWrSrcDataDual::width());
    REGISTERED_INPUT(pc_data, XLEN);
    REGISTERED_INPUT(alures_data, XLEN);

    // Valid signals for each way (not register clearing)
    REGISTERED_INPUT(exec_valid, 1);
    REGISTERED_INPUT(data_valid, 1);
};

}  // namespace core
}  // namespace vsrtl
