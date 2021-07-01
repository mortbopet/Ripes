#pragma once

#include "../riscv.h"
#include "../rv_decode.h"
#include "../rv_uncompress.h"
#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class DecodeRVC : public Component {
public:
    void setISA(const std::shared_ptr<ISAInfoBase>& isa) {
        decode->setISA(isa);
        uncompress->setISA(isa);
    }

    DecodeRVC(std::string name, SimComponent* parent) : Component(name, parent) {
        instr >> uncompress->instr;

        uncompress->Pc_Inc >> Pc_Inc;
        uncompress->exp_instr >> decode->instr;
        uncompress->exp_instr >> exp_instr;

        decode->opcode >> opcode;
        decode->wr_reg_idx >> wr_reg_idx;
        decode->r1_reg_idx >> r1_reg_idx;
        decode->r2_reg_idx >> r2_reg_idx;
    }

    SUBCOMPONENT(decode, TYPE(Decode<XLEN>));
    SUBCOMPONENT(uncompress, TYPE(Uncompress<XLEN>));

    INPUTPORT(instr, c_RVInstrWidth);
    OUTPUTPORT_ENUM(opcode, RVInstr);
    OUTPUTPORT(wr_reg_idx, c_RVRegsBits);
    OUTPUTPORT(r1_reg_idx, c_RVRegsBits);
    OUTPUTPORT(r2_reg_idx, c_RVRegsBits);
    OUTPUTPORT(Pc_Inc, 1);
    OUTPUTPORT(exp_instr, c_RVInstrWidth);
};

}  // namespace core
}  // namespace vsrtl
