#pragma once

#include "VSRTL/core/vsrtl_component.h"

namespace vsrtl {
namespace core {

template <unsigned INLEN, unsigned OUTLEN>
class ZExtOrTruncate : public Component {
public:
  ZExtOrTruncate(std::string name, SimComponent *parent)
      : Component(name, parent) {
    out << [this] { return in.uValue(); };
  }

  INPUTPORT(in, INLEN);
  OUTPUTPORT(out, OUTLEN);
};

} // namespace core
} // namespace vsrtl
