#pragma once

#include "isainfo.h"
#include "mipsisainfo_common.h"

namespace Ripes {

template <>
class ISAInfo<ISA::MIPS32I> : public MIPSISAInfoBase {
public:
  ISAInfo<ISA::MIPS32I>(const QStringList extensions) {
    // Validate extensions
    for (const auto &ext : extensions) {
      if (supportsExtension(ext)) {
        m_enabledExtensions << ext;
      }
    }
  }

  ISA isaID() const override { return ISA::MIPS32I; }

  unsigned int bits() const override { return 32; }
  unsigned elfMachineId() const override { return EM_MIPS; }
  QString CCmarch() const override {
    QString march = "mips32i";
    return march;
  }
  QString CCmabi() const override { return "ilp32"; }

  unsigned instrByteAlignment() const override { return 4; };
};

} // namespace Ripes
