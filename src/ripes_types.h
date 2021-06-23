#pragma once

#include <cstdint>
#include <type_traits>

namespace Ripes {

// Data types representing simulator memory addresses and simulator values. The type should be wide enough to represent
// all ISAs supported by Ripes. AInt and VInt are equal, but we discern between the two for code readability.
using AInt = uint64_t;                                // Address type
using VInt = uint64_t;                                // Value type
using AIntS = typename std::make_signed<AInt>::type;  // Signed address type
using VIntS = typename std::make_signed<VInt>::type;  // Signed value type

}  // namespace Ripes
