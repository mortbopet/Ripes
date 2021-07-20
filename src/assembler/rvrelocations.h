#pragma once

#include "relocation.h"
#include "vsrtl_binutils.h"

namespace Ripes {
namespace Assembler {

// pcrel_lo/hi are restricted to 32-bit absolute addresses, so keep computations in this base.
inline uint32_t pcrel_hi20(const uint32_t val, const uint32_t reloc_addr) {
    return ((val - (reloc_addr % 0xFFFFF000) + 0x800) >> 12);
}

template <typename Reg_T, typename Instr_T>
Relocation<Reg_T, Instr_T> rv_pcrel_hi() {
    return Relocation<Reg_T, Instr_T>("%pcrel_hi",
                                      [](const Reg_T val, const Reg_T reloc_addr) -> HandleRelocationRes<Reg_T> {
                                          const uint32_t _hi20 = pcrel_hi20(val, reloc_addr);
                                          return {_hi20};
                                      });
}

template <typename Reg_T, typename Instr_T>
Relocation<Reg_T, Instr_T> rv_pcrel_lo() {
    return Relocation<Reg_T, Instr_T>(
        "%pcrel_lo", [](const Reg_T val, const Reg_T reloc_addr) -> HandleRelocationRes<Reg_T> {
            using Reg_T_S = typename std::make_signed<Reg_T>::type;
            const uint32_t _hi20 = pcrel_hi20(val, reloc_addr);
            const uint32_t lo12 = val - (reloc_addr % 0xFFFFF000) - (_hi20 << 12);
            return {static_cast<Reg_T>(static_cast<Reg_T_S>(vsrtl::signextend(lo12, 12)))};
        });
}

/** @brief
 * A collection of RISC-V assembler relocations
 */

template <typename Reg_T, typename Instr_T>
RelocationsVec<Reg_T, Instr_T> rvRelocations() {
    RelocationsVec<Reg_T, Instr_T> relocations;

    relocations.push_back(std::make_shared<Relocation<Reg_T, Instr_T>>(rv_pcrel_hi<Reg_T, Instr_T>()));
    relocations.push_back(std::make_shared<Relocation<Reg_T, Instr_T>>(rv_pcrel_lo<Reg_T, Instr_T>()));

    return relocations;
}
}  // namespace Assembler
}  // namespace Ripes
