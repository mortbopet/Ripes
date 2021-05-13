#pragma once

#include "../rv5s/rv5s_idex.h"
#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief The RV5S_IDEX class
 * A specialization of the default IDEX stage separating register utilized by the rv5s_no_fw_hz processor. Storage of
 * register read indices is added, which are required by the forwarding unit.
 */
class RV5S_IDII_DUAL : public Component {
public:
    RV5S_IDII_DUAL(std::string name, SimComponent* parent) : Component(name, parent) {
        CONNECT_REGISTERED_CLEN_INPUT(pc_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(pc_exec, clear, enable);

        CONNECT_REGISTERED_CLEN_INPUT(pc4, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(pc, clear, enable);

        CONNECT_REGISTERED_CLEN_INPUT(rd_reg1_idx_exec, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(rd_reg2_idx_exec, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(opcode_exec, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(wr_reg_idx_exec, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(exec_valid, clear, enable);

        CONNECT_REGISTERED_CLEN_INPUT(rd_reg1_idx_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(rd_reg2_idx_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(opcode_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(wr_reg_idx_data, clear, enable);

        CONNECT_REGISTERED_CLEN_INPUT(way_stall, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(data_valid, clear, enable);

        CONNECT_REGISTERED_CLEN_INPUT(valid, clear, enable);

        CONNECT_REGISTERED_CLEN_INPUT(instr_data, clear, enable);
        CONNECT_REGISTERED_CLEN_INPUT(instr_exec, clear, enable);
    }

    REGISTERED_CLEN_INPUT(instr_data, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(instr_exec, RV_REG_WIDTH);

    REGISTERED_CLEN_INPUT(pc_data, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(pc_exec, RV_REG_WIDTH);

    REGISTERED_CLEN_INPUT(pc, RV_REG_WIDTH);
    REGISTERED_CLEN_INPUT(pc4, RV_REG_WIDTH);

    REGISTERED_CLEN_INPUT(rd_reg1_idx_exec, RV_REGS_BITS);
    REGISTERED_CLEN_INPUT(rd_reg2_idx_exec, RV_REGS_BITS);
    REGISTERED_CLEN_INPUT(opcode_exec, RVInstr::width());
    REGISTERED_CLEN_INPUT(wr_reg_idx_exec, RV_REGS_BITS);

    REGISTERED_CLEN_INPUT(rd_reg1_idx_data, RV_REGS_BITS);
    REGISTERED_CLEN_INPUT(rd_reg2_idx_data, RV_REGS_BITS);
    REGISTERED_CLEN_INPUT(opcode_data, RVInstr::width());
    REGISTERED_CLEN_INPUT(wr_reg_idx_data, RV_REGS_BITS);

    REGISTERED_CLEN_INPUT(way_stall, 1);

    REGISTERED_CLEN_INPUT(exec_valid, 1);
    REGISTERED_CLEN_INPUT(data_valid, 1);

    REGISTERED_CLEN_INPUT(valid, 1);

    // Register bank controls
    INPUTPORT(enable, 1);
    INPUTPORT(clear, 1);
};

}  // namespace core
}  // namespace vsrtl
