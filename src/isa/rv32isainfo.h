#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {

template <typename RVExtensions>
struct RV32ISAInterface
    : public RVISAInterface<RV32ISAInterface<RVExtensions>> {
  constexpr static ISA isaID() { return ISA::RV32I; }
  constexpr static unsigned int bits() { return 32; }
  constexpr static unsigned elfMachineId() { return EM_RISCV; }
  static QString CCmarch() {
    QString march = "rv32i";

    // Proceed in canonical order. Canonical ordering is defined in the RISC-V
    // spec.
    for (const auto &ext : {"M", "A", "F", "D", "C"}) {
      if (RV32ISAInterface::supportedExtensions().contains(ext)) {
        march += QString(ext).toLower();
      }
    }

    return march;
  }
  static QString CCmabi() { return "ilp32"; }

  unsigned instrByteAlignment() {
    return RVExtensions::extensionEnabled("C") ? 2 : 4;
  };
};

} // namespace Ripes
