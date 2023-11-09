#pragma once

#include "VSRTL/interface/vsrtl_binutils.h"
#include "isa/isa_defines.h"

namespace Ripes {
namespace Assembler {

// pcrel_lo/hi are restricted to 32-bit absolute addresses, so keep computations
// in this base.
inline uint32_t pcrel_hi20(const uint32_t val, const uint32_t reloc_addr) {
  return ((val - (reloc_addr % 0xFFFFF000) + 0x800) >> 12);
}

inline Relocation rv_pcrel_hi() {
  return Relocation(
      "%pcrel_hi",
      [](const Reg_T val, const Reg_T reloc_addr) -> HandleRelocationRes {
        const uint32_t _hi20 = pcrel_hi20(val, reloc_addr);
        return {_hi20};
      });
}

inline Relocation rv_pcrel_lo() {
  return Relocation(
      "%pcrel_lo",
      [](const Reg_T val, const Reg_T reloc_addr) -> HandleRelocationRes {
        using Reg_T_S = typename std::make_signed<Reg_T>::type;
        const uint32_t _hi20 = pcrel_hi20(val, reloc_addr);
        const uint32_t lo12 = val - (reloc_addr % 0xFFFFF000) - (_hi20 << 12);
        return {static_cast<Reg_T>(
            static_cast<Reg_T_S>(vsrtl::signextend(lo12, 12)))};
      });
}

inline Relocation rv_hi() {
  return Relocation(
      "%hi",
      [](const Reg_T val, const Reg_T /*reloc_addr*/) -> HandleRelocationRes {
        return {val >> 12 & 0xFFFFFF};
      });
}

inline Relocation rv_lo() {
  return Relocation(
      "%lo",
      [](const Reg_T val, const Reg_T /*reloc_addr*/) -> HandleRelocationRes {
        return {val & 0xFFF};
      });
}

/** @brief
 * A collection of RISC-V assembler relocations
 */

inline RelocationsVec rvRelocations() {
  RelocationsVec relocations;

  relocations.push_back(std::make_shared<Relocation>(rv_pcrel_hi()));
  relocations.push_back(std::make_shared<Relocation>(rv_pcrel_lo()));
  relocations.push_back(std::make_shared<Relocation>(rv_hi()));
  relocations.push_back(std::make_shared<Relocation>(rv_lo()));

  return relocations;
}
} // namespace Assembler
} // namespace Ripes
