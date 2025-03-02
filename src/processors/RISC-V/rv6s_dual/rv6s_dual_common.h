#pragma once

#include "processors/RISC-V/riscv.h"

namespace Ripes {
enum class RegWrSrcDual { ALURES, PC4 };
enum class RegWrSrcDataDual { ALURES, MEM };
enum class PcSrcDual { PC4, PC8 };
enum class WaySrc { WAY1, WAY2 };
} // namespace Ripes
