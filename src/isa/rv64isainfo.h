#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {

template <>
class ISAInfo<ISA::RV64I> : public RVISA::RV_ISAInfoBase {
public:
  ISAInfo<ISA::RV64I>(const QStringList extensions)
      : RV_ISAInfoBase(extensions) {}

  ISA isaID() const override { return ISA::RV64I; }

  unsigned int bits() const override { return 64; }
  QString CCmarch() const override {
    QString march = "rv64i";

    return _CCmarch(march);
  }
  QString CCmabi() const override { return "lp64"; }

  unsigned instrByteAlignment() const override {
    return extensionEnabled("C") ? 2 : 4;
  };
};

} // namespace Ripes
