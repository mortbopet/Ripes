#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_register.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <unsigned XLEN>
class RVMCPC : public Component {
public:
  RVMCPC(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    setDescription(
        "Register Program Counter");
    CONNECT_REGISTERED_CLEN_INPUT(pc, 0, enable);
  }

  REGISTERED_CLEN_INPUT(pc, XLEN);

  // Register controls
  INPUTPORT(enable, 1);
};

} // namespace core
} // namespace vsrtl
