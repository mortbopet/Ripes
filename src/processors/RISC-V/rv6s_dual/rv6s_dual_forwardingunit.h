#pragma once

#include "rv6s_dual_common.h"

#include "VSRTL/core/vsrtl_component.h"

namespace Ripes {
Enum(ForwardingSrcDual, IdStage, MemStageExec, MemStageMem, WbStageExec, WbStageMem);
}

namespace vsrtl {
namespace core {
using namespace Ripes;

class ForwardingUnit_DUAL : public Component {
    ForwardingSrcDual calculateForwarding(const VSRTL_VT_U& readidx) {
        if (readidx == 0) {
            return ForwardingSrcDual::IdStage;
        } else if (readidx == mem_reg_wr_idx_exec.uValue() && mem_reg_wr_en_exec.uValue()) {
            return ForwardingSrcDual::MemStageExec;
        } else if (readidx == mem_reg_wr_idx_data.uValue() && mem_reg_wr_en_data.uValue() &&
                   mem_reg_mem_op.uValue() == MemOp::NOP) {
            return ForwardingSrcDual::MemStageMem;
        } else if (readidx == wb_reg_wr_idx_exec.uValue() && wb_reg_wr_en_exec.uValue()) {
            return ForwardingSrcDual::WbStageExec;
        } else if (readidx == wb_reg_wr_idx_data.uValue() && wb_reg_wr_en_data.uValue()) {
            return ForwardingSrcDual::WbStageMem;
        }
        return ForwardingSrcDual::IdStage;
    }

public:
    ForwardingUnit_DUAL(const std::string& name, SimComponent* parent) : Component(name, parent) {
        alu_reg1_fw_ctrl_exec << [=] { return calculateForwarding(id_reg1_idx_exec.uValue()); };
        alu_reg2_fw_ctrl_exec << [=] { return calculateForwarding(id_reg2_idx_exec.uValue()); };
        alu_reg1_fw_ctrl_data << [=] { return calculateForwarding(id_reg1_idx_data.uValue()); };
        alu_reg2_fw_ctrl_data << [=] { return calculateForwarding(id_reg2_idx_data.uValue()); };
    }

    INPUTPORT(id_reg1_idx_data, c_RVRegsBits);
    INPUTPORT(id_reg2_idx_data, c_RVRegsBits);
    INPUTPORT(id_reg1_idx_exec, c_RVRegsBits);
    INPUTPORT(id_reg2_idx_exec, c_RVRegsBits);

    INPUTPORT(mem_reg_wr_idx_exec, c_RVRegsBits);
    INPUTPORT(mem_reg_wr_en_exec, 1);

    INPUTPORT(mem_reg_wr_idx_data, c_RVRegsBits);
    INPUTPORT(mem_reg_wr_en_data, 1);
    INPUTPORT_ENUM(mem_reg_mem_op, MemOp);

    INPUTPORT(wb_reg_wr_idx_exec, c_RVRegsBits);
    INPUTPORT(wb_reg_wr_en_exec, 1);

    INPUTPORT(wb_reg_wr_idx_data, c_RVRegsBits);
    INPUTPORT(wb_reg_wr_en_data, 1);

    OUTPUTPORT_ENUM(alu_reg1_fw_ctrl_exec, ForwardingSrcDual);
    OUTPUTPORT_ENUM(alu_reg2_fw_ctrl_exec, ForwardingSrcDual);

    OUTPUTPORT_ENUM(alu_reg1_fw_ctrl_data, ForwardingSrcDual);
    OUTPUTPORT_ENUM(alu_reg2_fw_ctrl_data, ForwardingSrcDual);
};
}  // namespace core
}  // namespace vsrtl
