#pragma once

#include "../riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace Ripes {
Enum(ForwardingSrc, IdStage, MemStage, WbStage);
}

namespace vsrtl {
namespace core {
using namespace Ripes;

class ForwardingUnit : public Component {
public:
    ForwardingUnit(std::string name, SimComponent* parent) : Component(name, parent) {
        alu_reg1_forwarding_ctrl << [=] {
            const auto idx = id_reg1_idx.uValue();
            if (idx == 0) {
                return ForwardingSrc::IdStage;
            } else if (idx == mem_reg_wr_idx.uValue() && mem_reg_wr_en.uValue()) {
                return ForwardingSrc::MemStage;
            } else if (idx == wb_reg_wr_idx.uValue() && wb_reg_wr_en.uValue()) {
                return ForwardingSrc::WbStage;
            } else {
                return ForwardingSrc::IdStage;
            }
        };

        alu_reg2_forwarding_ctrl << [=] {
            const auto idx = id_reg2_idx.uValue();
            if (idx == 0) {
                return ForwardingSrc::IdStage;
            } else if (idx == mem_reg_wr_idx.uValue() && mem_reg_wr_en.uValue()) {
                return ForwardingSrc::MemStage;
            } else if (idx == wb_reg_wr_idx.uValue() && wb_reg_wr_en.uValue()) {
                return ForwardingSrc::WbStage;
            } else {
                return ForwardingSrc::IdStage;
            }
        };
    }

    INPUTPORT(id_reg1_idx, RV_REGS_BITS);
    INPUTPORT(id_reg2_idx, RV_REGS_BITS);

    INPUTPORT(mem_reg_wr_idx, RV_REGS_BITS);
    INPUTPORT(mem_reg_wr_en, 1);

    INPUTPORT(wb_reg_wr_idx, RV_REGS_BITS);
    INPUTPORT(wb_reg_wr_en, 1);

    OUTPUTPORT_ENUM(alu_reg1_forwarding_ctrl, ForwardingSrc);
    OUTPUTPORT_ENUM(alu_reg2_forwarding_ctrl, ForwardingSrc);
};
}  // namespace core
}  // namespace vsrtl
