#pragma once

#include "rv5mc_base.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RV5MC2M : public RV5MCBase<XLEN_T> {
public:

  RV5MC2M(const QStringList &extensions)
    : RV5MCBase<XLEN_T>(extensions) {
  }
};

} // namespace core
} // namespace vsrtl
