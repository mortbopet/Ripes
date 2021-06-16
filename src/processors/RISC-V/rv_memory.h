#pragma once

#include "VSRTL/core/vsrtl_memory.h"
#include "VSRTL/core/vsrtl_wire.h"
#include "riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned int addrWidth, unsigned int dataWidth>
class RVMemory : public Component, public BaseMemory<true> {
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
                case MemOp::SD:
                    return 8;
                default:
                    return 0;
            }
        };
        wr_width->out >> mem->wr_width;

        data_out << [=] {
            switch (op.uValue()) {
                case MemOp::LB:
                    return VT_U(signextend<VSRTL_VT_U, 8>(mem->data_out.uValue() & 0xFFUL));
                case MemOp::LBU:
                    return mem->data_out.uValue() & 0xFFUL;
                case MemOp::LH:
                    return VT_U(signextend<VSRTL_VT_U, 16>(mem->data_out.uValue() & 0xFFFFUL));
                case MemOp::LHU:
                    return mem->data_out.uValue() & 0xFFFFUL;
                case MemOp::LW:
                    return mem->data_out.uValue() & 0xFFFFFFFFUL;
                case MemOp::LWU:
                    return VT_U(signextend<VSRTL_VT_U, 32>(mem->data_out.uValue() & 0xFFFFFFFFUL));
                case MemOp::LD:
                    return mem->data_out.uValue();
                default:
                    return mem->data_out.uValue();
            }
        };
    }

    void setMemory(AddressSpace* addressSpace) {
        setMemory(addressSpace);
        mem->setMemory(addressSpace);
    }

    // RVMemory is also a BaseMemory... A bit redundant, but RVMemory has a notion of the memory operation that is
    // happening, while the underlying MemoryAsyncRd does not.
    VSRTL_VT_U addressSig() const override { return addr.uValue(); };
    VSRTL_VT_U wrEnSig() const override { return wr_en.uValue(); };
    VSRTL_VT_U opSig() const override { return op.uValue(); };
    AddressSpace::RegionType accessRegion() const override { return mem->accessRegion(); }

    SUBCOMPONENT(mem, TYPE(MemoryAsyncRd<addrWidth, dataWidth>));

    WIRE(wr_width, ceillog2(dataWidth / 8 + 1));  // Write width, in bytes

    INPUTPORT(addr, addrWidth);
    INPUTPORT(data_in, dataWidth);
    INPUTPORT(wr_en, 1);
    INPUTPORT_ENUM(op, MemOp);
    OUTPUTPORT(data_out, dataWidth);
};

}  // namespace core
}  // namespace vsrtl
