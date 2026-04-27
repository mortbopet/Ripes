#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

#include "../rv5s_no_fw_hz/rv5s_no_fw_hz_exmem.h"

#include "../riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class EXMEM_F : public EXMEM<XLEN> {
public:
  EXMEM_F(const std::string &name, SimComponent *parent)
      : EXMEM<XLEN>(name, parent) {
    CONNECT_REGISTERED_CLEN_INPUT(regF_do_write, this->clear, this->enable);
  }
  // Control
  REGISTERED_CLEN_INPUT(regF_do_write, 1);
};

} // namespace core
} // namespace vsrtl
