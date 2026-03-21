#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {

template <>
class ISAInfo<ISA::RV64I> : public RVISA::RV_ISAInfoBase {
public:
  ISAInfo(const QStringList extensions) : RV_ISAInfoBase(extensions) {
    initialize(
        {RVISA::Option::shifts64BitVariant, RVISA::Option::LI64BitVariant});
  }

  ISA isaID() const override { return ISA::RV64I; }

  unsigned int bits() const override { return 64; }

  QString CCmabi() const override { 
    QString f = extensionEnabled(Extension::F) ? "f" : "";
    return "lp64"; 
  }

  unsigned instrByteAlignment() const override {
    return extensionEnabled("C") ? 2 : 4;
  };
};

} // namespace Ripes
