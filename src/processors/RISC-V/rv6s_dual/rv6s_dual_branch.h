#pragma once

#include "VSRTL/core/vsrtl_component.h"

#include "rv6s_dual_common.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class Branch_DUAL : public Component {
    /* clang-format off */
    bool branch_taken() const {
        switch(comp_op.uValue()){
            case CompOp::NOP: return false;
            case CompOp::EQ: return op1.uValue() == op2.uValue();
            case CompOp::NE: return op1.uValue() != op2.uValue();
            case CompOp::LT: return op1.sValue() < op2.sValue();
            case CompOp::LTU: return op1.uValue() < op2.uValue();
            case CompOp::GE: return op1.sValue() >= op2.sValue();
            case CompOp::GEU: return op1.uValue() >= op2.uValue();
            default: assert("Comparator: Unknown comparison operator"); return false;
        }
    }
    /* clang-format on */

    void computeCycle() {
        if (m_design->getCycleCount() == cachedCycle)
            return;

        m_did_controlflow = (branch_taken() && do_branch.uValue()) || do_jump.uValue();

        cachedCycle = m_design->getCycleCount();
    }

public:
    Branch_DUAL(const std::string& name, SimComponent* parent) : Component(name, parent) {
        pc_src << [=] {
            computeCycle();
            if (m_did_controlflow) {
                return PcSrc::ALU;
            } else {
                // Todo: Info from scheduling
                return PcSrc::PC4;
            }
        };

        did_controlflow << [=] {
            computeCycle();
            return m_did_controlflow;
        };
    }

    INPUTPORT_ENUM(comp_op, CompOp);
    INPUTPORT(op1, XLEN);
    INPUTPORT(op2, XLEN);
    INPUTPORT(do_branch, 1);
    INPUTPORT(do_jump, 1);
    OUTPUTPORT_ENUM(pc_src, PcSrc);
    OUTPUTPORT(did_controlflow, 1);

private:
    unsigned cachedCycle = -1;
    bool m_did_controlflow;
};

}  // namespace core
}  // namespace vsrtl
