#pragma once

#include "processors/RISC-V/rv5mc/rv5mc_base.h"
#include "processors/RISC-V/rv_memory.h"
#include "rv5mc_zextortruncate.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV5MC1M : public RV5MCBase<XLEN_T> {
  using FSMState = rv5mc::FSMState;
  using RVMCControl = rv5mc::RVMCControl;
  using AluSrc1 = rv5mc::AluSrc1;
  using AluSrc2 = rv5mc::AluSrc2;
  using ALUControl = rv5mc::ALUControl;
  using MemAddrSrc = rv5mc::MemAddrSrc;

public:

  RV5MC1M(const QStringList &extensions)
    : RV5MCBase<XLEN_T>(extensions) {
    // memory
    this->pc_reg->out >> this->mem_addr_src->get(MemAddrSrc::PC);
    this->ALU_out->out >> this->mem_addr_src->get(MemAddrSrc::ALUOUT);
    this->control->mem_addr_src >> this->mem_addr_src->select;
    this->mem_addr_src->out >> this->memory->addr;
    this->memory->data_out >> this->mem_out->in;
    this->mem_out->out >> this->reg_src->get(RegWrSrc::MEMREAD);
    this->control->mem_write >> this->memory->wr_en;
    this->b->out >> this->memory->data_in;
    this->control->mem_ctrl >> this->memory->op;
    this->memory->setMemory(this->m_memory);

    
    // Decode
    this->memory->data_out >> this->ir_widthadjust->in;
    this->ir_widthadjust->out >> this->decode->instr;
    this->decode->instr << [=] { return this->memory->data_out.uValue(); };
  }

  MemoryAccess dataMemAccess() const override {
    return this->memToAccessInfo(memory);
  }
  MemoryAccess instrMemAccess() const override {
    auto instrAccess = this->memToAccessInfo(memory);
    instrAccess.type = MemoryAccess::Read;
    return instrAccess;
  }

  // Memories
  SUBCOMPONENT(memory, TYPE(RVMemory<RV5MCBase<XLEN_T>::XLEN, RV5MCBase<XLEN_T>::XLEN>));

  SUBCOMPONENT(mem_addr_src, TYPE(EnumMultiplexer<MemAddrSrc, RV5MCBase<XLEN_T>::XLEN>));
  SUBCOMPONENT(ir_widthadjust, TYPE(ZExtOrTruncate<RV5MCBase<XLEN_T>::XLEN, c_RVInstrWidth>));
};

} // namespace core
} // namespace vsrtl
