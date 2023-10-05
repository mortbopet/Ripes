#pragma once

#include "VSRTL/interface/vsrtl_binutils.h"
#include "relocation.h"

namespace Ripes {
namespace Assembler {

// pcrel_lo/hi are restricted to 32-bit absolute addresses, so keep computations
// in this base.
inline uint32_t pcrel_hi20(const uint32_t val, const uint32_t reloc_addr) {
  return ((val - (reloc_addr % 0xFFFFF000) + 0x800) >> 12);
}

Relocation rv_pcrel_hi();

Relocation rv_pcrel_lo();

Relocation rv_hi();

Relocation rv_lo();

/** @brief
 * A collection of RISC-V assembler relocations
 */

RelocationsVec rvRelocations();

} // namespace Assembler
} // namespace Ripes
