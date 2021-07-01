#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class Branch : public Component {
public:
    Branch(const std::string& name, SimComponent* parent) : Component(name, parent) {
        // clang-format off
        res << [=] {
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
        };
        // clang-format on
    }

    INPUTPORT_ENUM(comp_op, CompOp);
    INPUTPORT(op1, XLEN);
    INPUTPORT(op2, XLEN);
    OUTPUTPORT(res, 1);
};

}  // namespace core
}  // namespace vsrtl
