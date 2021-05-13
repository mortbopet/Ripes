#pragma once

#include "../riscv.h"

namespace Ripes {
Enum(RegWrSrcDual, ALURES, PC4);
Enum(RegWrSrcDataDual, ALURES, MEM);
Enum(PcSrcDual, PC4, PC8);
Enum(WaySrc, WAY1, WAY2);
}  // namespace Ripes
