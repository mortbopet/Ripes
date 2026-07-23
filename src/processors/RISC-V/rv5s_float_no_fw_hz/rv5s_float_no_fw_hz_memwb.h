#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_memwb.h"

#include "processors/RISC-V/riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class MEMWB_F : public MEMWB<XLEN> {
public:
  MEMWB_F(const std::string &name, SimComponent *parent)
      : MEMWB<XLEN>(name, parent) {
    CONNECT_REGISTERED_INPUT(regF_do_write);
  }

  // Control
  REGISTERED_INPUT(regF_do_write, 1);
};

} // namespace core
} // namespace vsrtl
