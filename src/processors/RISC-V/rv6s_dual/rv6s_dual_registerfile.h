#pragma once

#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_memory.h"
#include "VSRTL/core/vsrtl_wire.h"

#include "../riscv.h"
#include "../rv_registerfile.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN, bool readBypass>
class RegisterFile_DUAL : public Component {
public:
    SetGraphicsType(ClockedComponent);
    RegisterFile_DUAL(const std::string& name, SimComponent* parent) : Component(name, parent) {
        // Way 1
        r1_1_addr >> rf_1->r1_addr;
        r2_1_addr >> rf_1->r2_addr;
        wr_1_addr >> rf_1->wr_addr;
        r1_1_out << [&] { return doReadBypass(r1_1_addr.uValue(), 1, rf_1); };
        r2_1_out << [&] { return doReadBypass(r2_1_addr.uValue(), 2, rf_1); };
        data_1_in >> rf_1->data_in;
        wr_1_en >> rf_1->wr_en;

        // Way 2
        r1_2_addr >> rf_2->r1_addr;
        r2_2_addr >> rf_2->r2_addr;
        wr_2_addr >> rf_2->wr_addr;
        r1_2_out << [&] { return doReadBypass(r1_2_addr.uValue(), 1, rf_2); };
        r2_2_out << [&] { return doReadBypass(r2_2_addr.uValue(), 2, rf_2); };
        data_2_in >> rf_2->data_in;
        wr_2_en >> rf_2->wr_en;
    }

    void setMemory(AddressSpace* mem) {
        m_memory = mem;
        // All memory components must point to the same memory
        rf_1->setMemory(mem);
        rf_2->setMemory(mem);
    }

    SUBCOMPONENT(rf_1, TYPE(RegisterFile<XLEN, readBypass>));
    SUBCOMPONENT(rf_2, TYPE(RegisterFile<XLEN, readBypass>));

    // Way 1
    INPUTPORT(r1_1_addr, c_RVRegsBits);
    INPUTPORT(r2_1_addr, c_RVRegsBits);
    INPUTPORT(wr_1_addr, c_RVRegsBits);
    OUTPUTPORT(r1_1_out, XLEN);
    OUTPUTPORT(r2_1_out, XLEN);
    INPUTPORT(data_1_in, XLEN);
    INPUTPORT(wr_1_en, 1);

    // Way 2
    INPUTPORT(r1_2_addr, c_RVRegsBits);
    INPUTPORT(r2_2_addr, c_RVRegsBits);
    INPUTPORT(wr_2_addr, c_RVRegsBits);
    OUTPUTPORT(r1_2_out, XLEN);
    OUTPUTPORT(r2_2_out, XLEN);
    INPUTPORT(data_2_in, XLEN);
    INPUTPORT(wr_2_en, 1);

    VSRTL_VT_U getRegister(unsigned i) { return m_memory->readMem(i << ceillog2(XLEN / CHAR_BIT), XLEN / CHAR_BIT); }

private:
    /**
     * Extra level of read bypassing to handle the fact that the RegisterFile class only bypasses with its internal
     * signals, and is unaware of the dual-ported register file.
     */
    VSRTL_VT_U doReadBypass(const VSRTL_VT_U reg_idx, const int portIndex, const RegisterFile<XLEN, readBypass>* rf) {
        if (reg_idx == 0)
            return static_cast<unsigned>(0);

        const unsigned wr_idx1 = wr_1_addr.uValue();
        const unsigned wr_idx2 = wr_2_addr.uValue();
        if (wr_1_en.uValue() && wr_idx1 == reg_idx) {
            return data_1_in.uValue();
        } else if (wr_2_en.uValue() && wr_idx2 == reg_idx) {
            return data_2_in.uValue();
        } else {
            return portIndex == 1 ? rf->r1_out.uValue() : rf->r2_out.uValue();
        }
    }

    AddressSpace* m_memory = nullptr;
};

}  // namespace core
}  // namespace vsrtl
