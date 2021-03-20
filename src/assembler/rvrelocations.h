#pragma once

#include "relocation.h"

namespace Ripes {
namespace Assembler {

/** @brief
 * A collection of RISC-V assembler relocations
 */

RelocationsVec rvRelocations();

Relocation rv_pcrel_hi();
Relocation rv_pcrel_lo();

}  // namespace Assembler
}  // namespace Ripes
