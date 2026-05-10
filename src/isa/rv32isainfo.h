#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {
using namespace RVISA;

template <>
class ISAInfo<ISA::RV32I> : public RVISA::RV_ISAInfoBase {
public:
  ISAInfo() : ISAInfo(RV_ExtensionSet()) {}
  ISAInfo(const ExtensionSetInfo &extensions) : RV_ISAInfoBase(extensions) {
    initialize(Extension::I);
  }

  ISA isaID() const override { return ISA::RV32I; }

  unsigned int bits() const override { return 32; }

  QString CCmabi() const override {
    QString f = extensionEnabled(Extension::F) ? "f" : "";
    return "ilp32" + f;
  }

  unsigned instrByteAlignment() const override {
    return extensionEnabled(Extension::C) ? 2 : 4;
  };
};

} // namespace Ripes
