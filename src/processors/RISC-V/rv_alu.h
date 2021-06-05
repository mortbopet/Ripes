#pragma once

#include <math.h>

#include "riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class ALU : public Component {
public:
    SetGraphicsType(ALU);
    ALU(std::string name, SimComponent* parent) : Component(name, parent) {
        res << [=] {
            switch (ctrl.uValue()) {
                case ALUOp::ADD:
                    return op1.uValue() + op2.uValue();
                case ALUOp::SUB:
                    return op1.uValue() - op2.uValue();
                case ALUOp::MUL:
                    return VT_U(op1.sValue() * op2.sValue());
                case ALUOp::MULH: {
                    const auto result = static_cast<int64_t>(op1.sValue()) * static_cast<int64_t>(op2.sValue());
                    return VT_U(result >> 32);
                }
                case ALUOp::MULHU: {
                    const auto result = static_cast<uint64_t>(op1.uValue()) * static_cast<uint64_t>(op2.uValue());
                    return VT_U(result >> 32);
                }
                case ALUOp::MULHSU: {
                    const int64_t result = static_cast<int64_t>(op1.sValue()) * static_cast<uint64_t>(op2.uValue());
                    return VT_U(result >> 32);
                }
                case ALUOp::DIV:
                    if (op2.sValue() == 0) {
                        return VT_U(-1);
                    } else if (op1.sValue() == static_cast<int64_t>(-(std::pow(2, 32 - 1))) && op2.sValue() == -1) {
                        // Overflow
                        return VT_U(0x80000000);  // -2^(32-1)
                    } else {
                        return VT_U(op1.sValue() / op2.sValue());
                    }

                case ALUOp::DIVU:
                    if (op2.sValue() == 0) {
                        return VT_U(std::pow(2, 32) - 1);
                    } else {
                        return op1.uValue() / op2.uValue();
                    }

                case ALUOp::REM:
                    if (op2.sValue() == 0) {
                        return op1.uValue();
                    } else if (op1.sValue() == static_cast<int64_t>(-(std::pow(2, 32 - 1))) && op2.sValue() == -1) {
                        // Overflow
                        return VT_U(0);
                    } else {
                        return VT_U(op1.sValue() % op2.sValue());
                    }

                case ALUOp::REMU:
                    if (op2.uValue() == 0) {
                        return op1.uValue();
                    } else {
                        return op1.uValue() % op2.uValue();
                    }

                case ALUOp::AND:
                    return op1.uValue() & op2.uValue();

                case ALUOp::OR:
                    return op1.uValue() | op2.uValue();

                case ALUOp::XOR:
                    return op1.uValue() ^ op2.uValue();

                case ALUOp::SL:
                    return op1.uValue() << op2.uValue();

                case ALUOp::SRA:
                    return VT_U(op1.sValue() >> op2.uValue());

                case ALUOp::SRL:
                    return op1.uValue() >> op2.uValue();

                case ALUOp::LUI:
                    return op2.uValue();

                case ALUOp::LT:
                    return VT_U(op1.sValue() < op2.sValue() ? 1 : 0);

                case ALUOp::LTU:
                    return VT_U(op1.uValue() < op2.uValue() ? 1 : 0);

                case ALUOp::NOP:
                    return VT_U(0xDEADBEEF);

                default:
                    throw std::runtime_error("Invalid ALU opcode");
            }
        };
    }

    INPUTPORT_ENUM(ctrl, ALUOp);
    INPUTPORT(op1, RV_REG_WIDTH);
    INPUTPORT(op2, RV_REG_WIDTH);

    OUTPUTPORT(res, RV_REG_WIDTH);
};

}  // namespace core
}  // namespace vsrtl
