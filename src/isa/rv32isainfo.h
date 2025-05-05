#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {

template <>
class ISAInfo<ISA::RV32I> : public RVISA::RV_ISAInfoBase {
public:
  ISAInfo(const QStringList extensions)
      : RV_ISAInfoBase(extensions) {
    initialize();
  }

  ISA isaID() const override { return ISA::RV32I; }

  unsigned int bits() const override { return 32; }
  unsigned elfMachineId() const override { return EM_RISCV; }
  QString CCmarch() const override {
    QString march = "rv32i";

    return _CCmarch(march);
  }
  QString CCmabi() const override { return "ilp32"; }

  unsigned instrByteAlignment() const override {
    return extensionEnabled("C") ? 2 : 4;
  };
};

} // namespace Ripes
