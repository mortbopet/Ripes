#pragma once

#include "rv5s_vliw_common.h"

#include "VSRTL/core/vsrtl_component.h"

/* Note:
    We could have used the Forwarding unit introduced in the 6 stage dual
    issue processor but we don't facilitate the MemStageMem forwarding in
    this design. Therefor we only need 4 forwarding states instead of 5,
    reducing the Mux select bit width from 3 to 2 also reducing the possible
    hardware implementation of these Mux's.
*/

namespace Ripes {
enum class ForwardSrcVliw { IdStage, MemStageExec, WbStageExec, WbStageData };
}

namespace vsrtl {
namespace core {
using namespace Ripes;

class ForwardingUnit_VLIW : public Component {
  ForwardSrcVliw calculateForwarding(const VSRTL_VT_U &readidx) {
    if (readidx == 0) {
      return ForwardSrcVliw::IdStage;
    }

    if (readidx == mem_reg_wr_idx_exec.uValue() &&
        mem_reg_wr_en_exec.uValue()) {
      return ForwardSrcVliw::MemStageExec;
    }

    if (readidx == wb_reg_wr_idx_data.uValue() && wb_reg_wr_en_data.uValue()) {
      return ForwardSrcVliw::WbStageData;
    }

    if (readidx == wb_reg_wr_idx_exec.uValue() && wb_reg_wr_en_exec.uValue()) {
      return ForwardSrcVliw::WbStageExec;
    }

    return ForwardSrcVliw::IdStage;
  }

public:
  ForwardingUnit_VLIW(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    alu_exec_reg1_fw_ctrl <<
        [this] { return calculateForwarding(id_reg1_idx_exec.uValue()); };
    alu_exec_reg2_fw_ctrl <<
        [this] { return calculateForwarding(id_reg2_idx_exec.uValue()); };
    alu_data_reg1_fw_ctrl <<
        [this] { return calculateForwarding(id_reg1_idx_data.uValue()); };
    mem_data_fw_ctrl <<
        [this] { return calculateForwarding(id_reg2_idx_data.uValue()); };
  }

  INPUTPORT(id_reg1_idx_exec, c_RVRegsBits);
  INPUTPORT(id_reg2_idx_exec, c_RVRegsBits);
  INPUTPORT(id_reg1_idx_data, c_RVRegsBits);
  INPUTPORT(id_reg2_idx_data, c_RVRegsBits);

  INPUTPORT(mem_reg_wr_idx_exec, c_RVRegsBits);
  INPUTPORT(mem_reg_wr_en_exec, 1);

  INPUTPORT(wb_reg_wr_idx_exec, c_RVRegsBits);
  INPUTPORT(wb_reg_wr_en_exec, 1);

  INPUTPORT(wb_reg_wr_idx_data, c_RVRegsBits);
  INPUTPORT(wb_reg_wr_en_data, 1);

  OUTPUTPORT_ENUM(alu_exec_reg1_fw_ctrl, ForwardSrcVliw);
  OUTPUTPORT_ENUM(alu_exec_reg2_fw_ctrl, ForwardSrcVliw);

  OUTPUTPORT_ENUM(alu_data_reg1_fw_ctrl, ForwardSrcVliw);

  OUTPUTPORT_ENUM(mem_data_fw_ctrl, ForwardSrcVliw);
};
} // namespace core
} // namespace vsrtl
