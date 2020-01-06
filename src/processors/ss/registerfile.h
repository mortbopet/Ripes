#pragma once

#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_memory.h"
#include "VSRTL/core/vsrtl_wire.h"

#include "riscv.h"

namespace vsrtl {
using namespace core;
namespace RISCV {

class RegisterFile : public Component {
public:
    SetGraphicsType(ClockedComponent);
    RegisterFile(std::string name, SimComponent* parent) : Component(name, parent) {
        // Writes

        // Disable writes to register 0
        wr_en_0->setSensitiveTo(wr_en);
        wr_en_0->out << [=] { return wr_en.uValue() && wr_addr.uValue() != 0; };

        wr_addr >> _wr_mem->addr;
        wr_en_0->out >> _wr_mem->wr_en;
        data_in >> _wr_mem->data_in;
        4 >> _wr_mem->wr_width;

        // Reads
        r1_addr >> _rd1_mem->addr;
        r2_addr >> _rd2_mem->addr;
        _rd1_mem->data_out >> r1_out;
        _rd2_mem->data_out >> r2_out;
    }

    SUBCOMPONENT(_wr_mem, TYPE(WrMemory<RV_REGS_BITS, RV_REG_WIDTH, false>));
    SUBCOMPONENT(_rd1_mem, TYPE(RdMemory<RV_REGS_BITS, RV_REG_WIDTH, false>));
    SUBCOMPONENT(_rd2_mem, TYPE(RdMemory<RV_REGS_BITS, RV_REG_WIDTH, false>));

    INPUTPORT(r1_addr, RV_REGS_BITS);
    INPUTPORT(r2_addr, RV_REGS_BITS);
    INPUTPORT(wr_addr, RV_REGS_BITS);

    INPUTPORT(data_in, RV_REG_WIDTH);
    WIRE(wr_en_0, 1);
    INPUTPORT(wr_en, 1);
    OUTPUTPORT(r1_out, RV_REG_WIDTH);
    OUTPUTPORT(r2_out, RV_REG_WIDTH);

    VSRTL_VT_U getRegister(unsigned i) { return m_memory->readMem<false>(i); }

    std::vector<VSRTL_VT_U> getRegisters() {
        std::vector<VSRTL_VT_U> regs;
        for (int i = 0; i < RV_REGS; i++)
            regs.push_back(getRegister(i));
        return regs;
    }

    void setMemory(SparseArray* mem) {
        m_memory = mem;
        // All memory components must point to the same memory
        _wr_mem->setMemory(m_memory);
        _rd1_mem->setMemory(m_memory);
        _rd2_mem->setMemory(m_memory);
    }

private:
    SparseArray* m_memory = nullptr;
};

}  // namespace RISCV
}  // namespace vsrtl
