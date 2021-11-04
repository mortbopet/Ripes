#pragma once

#include "../riscv.h"
#include "../rv_uncompress.h"
#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class ShiftR16 : public Component {
public:
    ShiftR16(std::string name, SimComponent* parent) : Component(name, parent) {
        instr16 << [=] { return (((instr2.uValue() & 0xFFFF) << 16) | (instr1.uValue() >> 16)); };
    }

    INPUTPORT(instr1, XLEN);
    INPUTPORT(instr2, XLEN);
    OUTPUTPORT(instr16, XLEN);
};

template <unsigned XLEN>
class UncompressDual : public Component {
public:
    void setISA(const std::shared_ptr<ISAInfoBase>& isa) {
        uncompress1->setISA(isa);
        uncompress2->setISA(isa);
    }

    UncompressDual(std::string name, SimComponent* parent) : Component(name, parent) {
        setDescription("Uncompresses instructions from the 'C' extension into their 32-bit representation.");
        instr1 >> shiftr16->instr1;
        instr2 >> shiftr16->instr2;
        shiftr16->instr16 >> instr_split->get(PcInc::INC2);
        instr2 >> instr_split->get(PcInc::INC4);
        uncompress1->Pc_Inc >> instr_split->select;

        instr1 >> uncompress1->instr;
        uncompress1->Pc_Inc >> Pc_Inc1;
        uncompress1->exp_instr >> exp_instr1;

        instr_split->out >> uncompress2->instr;
        uncompress2->Pc_Inc >> Pc_Inc2;
        uncompress2->exp_instr >> exp_instr2;
    }

    SUBCOMPONENT(uncompress1, TYPE(Uncompress<XLEN>));
    SUBCOMPONENT(uncompress2, TYPE(Uncompress<XLEN>));
    SUBCOMPONENT(instr_split, TYPE(EnumMultiplexer<PcInc, c_RVInstrWidth>));
    SUBCOMPONENT(shiftr16, TYPE(ShiftR16<c_RVInstrWidth>));

    INPUTPORT(instr1, c_RVInstrWidth);
    INPUTPORT(instr2, c_RVInstrWidth);
    OUTPUTPORT(Pc_Inc1, 1);
    OUTPUTPORT(Pc_Inc2, 1);
    OUTPUTPORT(exp_instr1, c_RVInstrWidth);
    OUTPUTPORT(exp_instr2, c_RVInstrWidth);
};

}  // namespace core
}  // namespace vsrtl
