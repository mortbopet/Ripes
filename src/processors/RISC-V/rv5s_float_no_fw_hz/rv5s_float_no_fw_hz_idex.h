#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_idex.h"

#include "processors/RISC-V/riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class IDEX_F : public IDEX<XLEN> {
public:
  IDEX_F(const std::string &name, SimComponent *parent)
      : IDEX<XLEN>(name, parent) {
    CONNECT_REGISTERED_CLEN_INPUT(r2_mem, this->clear, this->enable);
    CONNECT_REGISTERED_CLEN_INPUT(f1, this->clear, this->enable);
    CONNECT_REGISTERED_CLEN_INPUT(f2, this->clear, this->enable);
    CONNECT_REGISTERED_CLEN_INPUT(f3, this->clear, this->enable);

    CONNECT_REGISTERED_CLEN_INPUT(alu_fpu_src, this->clear, this->enable);
    CONNECT_REGISTERED_CLEN_INPUT(regF_do_write, this->clear, this->enable);
    CONNECT_REGISTERED_CLEN_INPUT(fpu_ctrl, this->clear, this->enable);
    CONNECT_REGISTERED_CLEN_INPUT(roundMode, this->clear, this->enable);
  }

  // Data
  REGISTERED_CLEN_INPUT(r2_mem, XLEN);
  REGISTERED_CLEN_INPUT(f1, XLEN);
  REGISTERED_CLEN_INPUT(f2, XLEN);
  REGISTERED_CLEN_INPUT(f3, XLEN);

  // Control
  REGISTERED_CLEN_INPUT(alu_fpu_src, enumBitWidth<RegFileSrc>());
  REGISTERED_CLEN_INPUT(regF_do_write, 1);
  REGISTERED_CLEN_INPUT(fpu_ctrl, enumBitWidth<FPUOp>());
  REGISTERED_CLEN_INPUT(roundMode, enumBitWidth<RVISA::ExtF::RoundMode>());
};

} // namespace core
} // namespace vsrtl
