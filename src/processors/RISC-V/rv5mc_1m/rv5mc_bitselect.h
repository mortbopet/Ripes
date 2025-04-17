#pragma once

#include "processors/RISC-V/riscv.h"

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
using namespace Ripes;
  
template <unsigned INLEN, unsigned OUTLEN>
class BitSelectLSBs : public Component {
public:
  BitSelectLSBs(std::string name, SimComponent *parent) : Component(name, parent) {
    out << [=]{ return in.uValue(); };
  }
  
  INPUTPORT(in, INLEN);
  OUTPUTPORT(out, OUTLEN);
};
  
} // namespace core
} // namespace vsrtl
