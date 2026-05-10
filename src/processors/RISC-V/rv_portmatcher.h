#pragma once

#include "VSRTL/core/vsrtl_component.h"
#include "VSRTL/core/vsrtl_wire.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

/**
 * @brief Bit Width Matcher for Port bound inputs.
 *
 * @tparam inWidth input bit width to match from
 * @tparam outWidth output bit width to match to
 */
template <unsigned inWidth, unsigned outWidth>
class Ported_Matcher : public Component {
public:
  SetGraphicsType(Wire);
  Ported_Matcher(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    out <<
        [this] { return VT_U(in.uValue() & vsrtl::generateBitmask(outWidth)); };
  }

  INPUTPORT(in, inWidth);
  OUTPUTPORT(out, outWidth);
};

/**
 * @brief Bit Width Matcher for Function bound inputs.
 *
 * @note Due to the internal propagation mechanism of VSRTL, 'in' must be
 * implemented as an output port otherwise function bindings will not get
 * propagated correctly. This design is similar to the existing WIRE macro.
 * This however requires that the bindings to 'in' must be accompanied by
 * a sensitivity list inclusion via *setSensitiveTo(...)*.
 *
 * @tparam inWidth input bit width to match from
 * @tparam outWidth output bit width to match to
 */
template <unsigned inWidth, unsigned outWidth>
class Functioned_Matcher : public Component {
public:
  SetGraphicsType(Wire);
  Functioned_Matcher(const std::string &name, SimComponent *parent)
      : Component(name, parent) {
    out <<
        [this] { return VT_U(in.uValue() & vsrtl::generateBitmask(outWidth)); };
  }

  OUTPUTPORT(in, inWidth);
  OUTPUTPORT(out, outWidth);
};

} // namespace core
} // namespace vsrtl
