#pragma once

#include <cstdint>
#include <type_traits>

namespace Ripes {

// Data types representing simulator memory addresses and simulator values. The type should be wide enough to represent
// all ISAs supported by Ripes.
using AInt = uint64_t;
using VInt = uint64_t;
using VIntS = typename std::make_signed<VInt>::type;

}  // namespace Ripes
