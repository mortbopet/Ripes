#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"
#include "processors/RISC-V/riscv.h"
#include "processors/RISC-V/rv_decode.h"
#include "processors/RISC-V/rv_uncompress.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class DecodeRVMC : public Component {
public:
  void setISA(const std::shared_ptr<ISAInfoBase> &isa) {
    decode->setISA(isa);
    uncompress->setISA(isa);
  }

  DecodeRVMC(std::string name, SimComponent *parent) : Component(name, parent) {
    instr >> regIntr->in;
    0 >> regIntr->clear;
    enable >> regIntr->enable;

    regIntr->out >> uncompress->instr;

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

  SUBCOMPONENT(regIntr, RegisterClEn<c_RVInstrWidth>);

  INPUTPORT(instr, c_RVInstrWidth);

  INPUTPORT(enable,1);

  OUTPUTPORT_ENUM(opcode, RVInstr);
  OUTPUTPORT(wr_reg_idx, c_RVRegsBits);
  OUTPUTPORT(r1_reg_idx, c_RVRegsBits);
  OUTPUTPORT(r2_reg_idx, c_RVRegsBits);
  OUTPUTPORT(Pc_Inc, 1);
  OUTPUTPORT(exp_instr, c_RVInstrWidth);
};

} // namespace core
} // namespace vsrtl
