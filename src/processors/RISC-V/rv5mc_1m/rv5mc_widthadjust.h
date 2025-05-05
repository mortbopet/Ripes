#pragma once

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {
  
template <unsigned INLEN, unsigned OUTLEN>
class WidthAdjust : public Component {
public:
  WidthAdjust(std::string name, SimComponent *parent) : Component(name, parent) {
    out << [=]{ return in.uValue(); };
  }
  
  INPUTPORT(in, INLEN);
  OUTPUTPORT(out, OUTLEN);
};
  
} // namespace core
} // namespace vsrtl
