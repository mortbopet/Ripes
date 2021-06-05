#pragma once

#include "VSRTL/core/vsrtl_memory.h"
#include "VSRTL/core/vsrtl_wire.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned int addrWidth, unsigned int dataWidth>
class RVMemory : public Component {
public:
    SetGraphicsType(ClockedComponent);
    RVMemory(std::string name, SimComponent* parent) : Component(name, parent) {
        addr >> mem->addr;
        wr_en >> mem->wr_en;
        data_in >> mem->data_in;

        wr_width->setSensitiveTo(&op);
        wr_width->out << [=] {
            switch (op.uValue()) {
                case MemOp::SB:
                    return 1;
                case MemOp::SH:
                    return 2;
                case MemOp::SW:
                    return 4;
                default:
                    return 0;
            }
        };
        wr_width->out >> mem->wr_width;

        data_out << [=] {
            switch (op.uValue()) {
                case MemOp::LB:
                    return VT_U(signextend<int32_t, 8>(mem->data_out.uValue() & 0xFF));
                case MemOp::LBU:
                    return mem->data_out.uValue() & 0xFF;
                case MemOp::LH:
                    return VT_U(signextend<int32_t, 16>(mem->data_out.uValue() & 0xFFFF));
                case MemOp::LHU:
                    return mem->data_out.uValue() & 0xFFFF;
                case MemOp::LW:
                    return mem->data_out.uValue();
                default:
                    return mem->data_out.uValue();
            }
        };
    }

    SUBCOMPONENT(mem, TYPE(MemoryAsyncRd<RV_REG_WIDTH, RV_REG_WIDTH>));

    WIRE(wr_width, ceillog2(RV_REG_WIDTH / 8 + 1));

    INPUTPORT(addr, addrWidth);
    INPUTPORT(data_in, dataWidth);
    INPUTPORT(wr_en, 1);
    INPUTPORT_ENUM(op, MemOp);
    OUTPUTPORT(data_out, dataWidth);
};

}  // namespace core
}  // namespace vsrtl
