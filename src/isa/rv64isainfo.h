#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {
using namespace RVISA;

template <>
class ISAInfo<ISA::RV64I> : public RVISA::RV_ISAInfoBase {
public:
  ISAInfo() : ISAInfo(RV_ExtensionSet()) {}
  ISAInfo(const ExtensionSetInfo& extensions) : RV_ISAInfoBase(extensions) {
    initialize(
      Extension::I,
      {RVISA::Option::shifts64BitVariant, RVISA::Option::LI64BitVariant}
    );
  }

  ISA isaID() const override { return ISA::RV64I; }

  unsigned int bits() const override { return 64; }

  QString CCmabi() const override { 
    QString f = extensionEnabled(Extension::F) ? "f" : "";
    return "lp64"; 
  }

  unsigned instrByteAlignment() const override {
    return extensionEnabled(Extension::C) ? 2 : 4;
  };
};

} // namespace Ripes
