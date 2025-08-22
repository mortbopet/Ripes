#pragma once

#include "processors/RISC-V/rv_memory.h"
#include "rv5mc_base.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV5MC2M : public RV5MCBase<XLEN_T> {
  using FSMState = rv5mc::FSMState;
  using RVMCControl = rv5mc::RVMCControl;
  using AluSrc1 = rv5mc::AluSrc1;
  using AluSrc2 = rv5mc::AluSrc2;
  using ALUControl = rv5mc::ALUControl;
  using MemAddrSrc = rv5mc::MemAddrSrc;

public:
  RV5MC2M(const QStringList &extensions) : RV5MCBase<XLEN_T>(extensions) {
    // Instruction memory
    this->pc_reg->out >> this->instr_mem->addr;
    this->instr_mem->setMemory(this->m_memory);

    // Decode
    this->instr_mem->data_out >> this->decode->instr;

    // Data memory
    this->data_mem->data_out >> this->mem_out->in;
    this->mem_out->out >> this->reg_src->get(RegWrSrc::MEMREAD);
    this->ALU_out->out >> this->data_mem->addr;
    this->control->mem_write >> this->data_mem->wr_en;
    this->b->out >> this->data_mem->data_in;
    this->control->mem_ctrl >> this->data_mem->op;
    this->data_mem->setMemory(this->m_memory);
  }

  MemoryAccess dataMemAccess() const override {
    return this->memToAccessInfo(data_mem);
  }
  MemoryAccess instrMemAccess() const override {
    auto instrAccess = this->memToAccessInfo(instr_mem);
    instrAccess.type = MemoryAccess::Read;
    return instrAccess;
  }

  // Memories
  SUBCOMPONENT(instr_mem, TYPE(ROM<RV5MCBase<XLEN_T>::XLEN, c_RVInstrWidth>));
  SUBCOMPONENT(
      data_mem,
      TYPE(RVMemory<RV5MCBase<XLEN_T>::XLEN, RV5MCBase<XLEN_T>::XLEN>));
};

} // namespace core
} // namespace vsrtl
