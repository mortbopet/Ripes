#pragma once

#include <math.h>
#include "limits.h"

#include "riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

constexpr int32_t div_overflow32 = (-2147483648);  //-2^(32-1)
constexpr int64_t div_overflow64 = (LLONG_MIN);    //-2^(64-1)

template <unsigned XLEN>
class ALU : public Component {
public:
    SetGraphicsType(ALU);
    ALU(const std::string& name, SimComponent* parent) : Component(name, parent) {
        res << [=] {
            switch (ctrl.uValue()) {
                case ALUOp::ADD:
                    return op1.uValue() + op2.uValue();
                case ALUOp::SUB:
                    return op1.uValue() - op2.uValue();
                case ALUOp::MULW:
                    return VT_U(signextend<32>(static_cast<int32_t>(op1.uValue()) * op2.uValue()));
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

                case ALUOp::DIVW:
                case ALUOp::DIV: {
                    const VSRTL_VT_S overflow =
                        (ctrl.uValue() == ALUOp::DIVW) || (ctrl.uValue() == ALUOp::DIV && XLEN == 32) ? div_overflow32
                                                                                                      : div_overflow64;
                    if (op2.sValue() == 0) {
                        return VT_U(-1);
                    } else if (op1.sValue() == overflow && op2.sValue() == -1) {
                        // Overflow
                        return VT_U(overflow);
                    } else {
                        return VT_U(op1.sValue() / op2.sValue());
                    }
                }

                case ALUOp::DIVUW:
                case ALUOp::DIVU: {
                    if (op2.uValue() == 0) {
                        return VT_U(-1LL);
                    } else {
                        return op1.uValue() / op2.uValue();
                    }
                }

                case ALUOp::REMW:
                case ALUOp::REM: {
                    const VSRTL_VT_S overflow =
                        (ctrl.uValue() == ALUOp::REMW) || (ctrl.uValue() == ALUOp::REM && XLEN == 32) ? div_overflow32
                                                                                                      : div_overflow64;
                    if (op2.sValue() == 0) {
                        return op1.uValue();
                    } else if (op1.sValue() == overflow && op2.sValue() == -1) {
                        // Overflow
                        return VT_U(0);
                    } else {
                        return VT_U(op1.sValue() % op2.sValue());
                    }
                }

                case ALUOp::REMUW:
                case ALUOp::REMU: {
                    const VSRTL_VT_S overflow =
                        (ctrl.uValue() == ALUOp::REMUW) || (ctrl.uValue() == ALUOp::REMU && XLEN == 32)
                            ? div_overflow32
                            : div_overflow64;
                    if (op2.uValue() == 0) {
                        return op1.uValue();
                    } else {
                        return op1.uValue() % op2.uValue();
                    }

                    if (op2.sValue() == 0) {
                        return VT_U(-1);
                    } else if (op1.sValue() == overflow && op2.sValue() == -1) {
                        // Overflow
                        return VT_U(overflow);
                    } else {
                        return VT_U(op1.sValue() / op2.sValue());
                    }
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
                    return VT_U(signextend<32>(op2.uValue()));

                case ALUOp::LT:
                    return VT_U(op1.sValue() < op2.sValue() ? 1 : 0);

                case ALUOp::LTU:
                    return VT_U(op1.uValue() < op2.uValue() ? 1 : 0);

                case ALUOp::NOP:
                    return VT_U(0xDEADBEEF);

                case ALUOp::ADDW:
                    return VT_U(signextend<32>(op1.uValue() + op2.uValue()));

                case ALUOp::SUBW:
                    return VT_U(signextend<32>(op1.uValue() - op2.uValue()));

                case ALUOp::SLW:
                    return VT_U(signextend<32>(op1.uValue() << (op2.uValue() & generateBitmask(5))));

                case ALUOp::SRAW:
                    return VT_U(
                        signextend<32>(static_cast<int32_t>(op1.uValue()) >> (op2.uValue() & generateBitmask(5))));

                case ALUOp::SRLW:
                    return VT_U(
                        signextend<32>(static_cast<uint32_t>(op1.uValue()) >> (op2.uValue() & generateBitmask(5))));

                default:
                    throw std::runtime_error("Invalid ALU opcode");
            }
        };
    }

    INPUTPORT_ENUM(ctrl, ALUOp);
    INPUTPORT(op1, XLEN);
    INPUTPORT(op2, XLEN);

    OUTPUTPORT(res, XLEN);
};

}  // namespace core
}  // namespace vsrtl
