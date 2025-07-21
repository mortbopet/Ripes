#pragma once

#include "limits.h"
#include <math.h>

#include "processors/RISC-V/riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class RVMCALU : public Component {
private:
  long unsigned int carry_overflow32 = 2147483648;           // 2^(32-1)
  unsigned long int carry_overflow64 = 9223372036854775808U; // 2^(64-1)
public:
  SetGraphicsType(ALU);

  RVMCALU(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    res << [=] {
      switch (ctrl.eValue<ALUOp>()) {
      case ALUOp::ADD:
        return op1.uValue() + op2.uValue();
      case ALUOp::SUB:
        return op1.uValue() - op2.uValue();
      case ALUOp::MULW:
        return VT_U(
            signextend<32>(static_cast<int32_t>(op1.uValue()) * op2.uValue()));
      case ALUOp::MUL:
        return VT_U(op1.sValue() * op2.sValue());
      case ALUOp::MULH: {
        const auto result = static_cast<int64_t>(op1.sValue()) *
                            static_cast<int64_t>(op2.sValue());
        return VT_U(result >> 32);
      }
      case ALUOp::MULHU: {
        const auto result = static_cast<uint64_t>(op1.uValue()) *
                            static_cast<uint64_t>(op2.uValue());
        return VT_U(result >> 32);
      }
      case ALUOp::MULHSU: {
        const int64_t result = static_cast<int64_t>(op1.sValue()) *
                               static_cast<uint64_t>(op2.uValue());
        return VT_U(result >> 32);
      }

      case ALUOp::DIVW:
      case ALUOp::DIV: {
        const VSRTL_VT_S overflow =
            (ctrl.eValue<ALUOp>() == ALUOp::DIVW) ||
                    (ctrl.eValue<ALUOp>() == ALUOp::DIV && XLEN == 32)
                ? div_overflow32
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
            (ctrl.eValue<ALUOp>() == ALUOp::REMW) ||
                    (ctrl.eValue<ALUOp>() == ALUOp::REM && XLEN == 32)
                ? div_overflow32
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
            (ctrl.eValue<ALUOp>() == ALUOp::REMUW) ||
                    (ctrl.eValue<ALUOp>() == ALUOp::REMU && XLEN == 32)
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
        return VT_U(signextend<32>(op1.uValue()
                                   << (op2.uValue() & generateBitmask(5))));

      case ALUOp::SRAW:
        return VT_U(signextend<32>(static_cast<int32_t>(op1.uValue()) >>
                                   (op2.uValue() & generateBitmask(5))));

      case ALUOp::SRLW:
        return VT_U(signextend<32>(static_cast<uint32_t>(op1.uValue()) >>
                                   (op2.uValue() & generateBitmask(5))));

      default:
        throw std::runtime_error("Invalid ALU opcode");
      }
    };
    zero << [=] {
      if (res.uValue() == 0)
        return 1;
      return 0;
    }; // returns 1 if the result of the ALU is 0
    sign << [=] {
      return VT_U(res.sValue() < 0 ? 1 : 0);
    }; // returns 1 if the ALU result is negative or 0 if it is not
    carry << [=] {
      switch (ctrl.eValue<ALUOp>()) {
      case ALUOp::ADD:
        if (XLEN == 32)
          if ((op1.uValue() + op2.uValue()) > carry_overflow32)
            return 1;
        if (XLEN == 64)
          if ((op1.uValue() + op2.uValue()) > carry_overflow64)
            return 1;
        return 0;
      case ALUOp::SUB:
        if (op1.uValue() < op2.uValue())
          return 1;
        return 0;
      default:
        return 0;
      }
    };
  }

  INPUTPORT_ENUM(ctrl, ALUOp);
  INPUTPORT(op1, XLEN);
  INPUTPORT(op2, XLEN);

  OUTPUTPORT(res, XLEN);
  OUTPUTPORT(zero, 1);
  OUTPUTPORT(sign, 1);
  OUTPUTPORT(carry, 1);
};

} // namespace core
} // namespace vsrtl
