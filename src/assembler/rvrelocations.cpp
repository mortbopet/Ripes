#include "rvrelocations.h"

namespace Ripes {
namespace Assembler {

#define add_relocation(container, relocation) container.push_back(std::make_shared<Relocation>(relocation));

RelocationsVec rvRelocations() {
    RelocationsVec relocations;

    add_relocation(relocations, rv_pcrel_hi());
    add_relocation(relocations, rv_pcrel_lo());

    return relocations;
}

uint32_t hi20(const uint32_t symbol_address, const uint32_t reloc_addr) {
    return ((symbol_address - (reloc_addr % 0xFFFFF000) + 0x800) >> 12);
}

Relocation rv_pcrel_hi() {
    return Relocation("%pcrel_hi", [](const uint32_t symbol_address, const uint32_t reloc_addr) -> HandleRelocationRes {
        const uint32_t _hi20 = hi20(symbol_address, reloc_addr);
        return {_hi20};
    });
}
Relocation rv_pcrel_lo() {
    return Relocation("%pcrel_lo", [](const uint32_t symbol_address, const uint32_t reloc_addr) -> HandleRelocationRes {
        const uint32_t _hi20 = hi20(symbol_address, reloc_addr);
        const uint32_t lo12 = symbol_address - (reloc_addr % 0xFFFFF000) - (_hi20 << 12);
        return {lo12};
    });
}

}  // namespace Assembler
}  // namespace Ripes
