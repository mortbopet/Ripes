#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

#include "VSRTL/interface/vsrtl_binutils.h"
#include "limits.h"
#include "ripes_types.h"

/// Most of the code in this file originates from LLVM MathExtras.h

namespace Ripes {

/// Checks if an integer fits into the given bit width.
template <unsigned N>
constexpr inline bool isInt(VIntS x) {
    return N >= 64 || (-(INT64_C(1) << (N - 1)) <= x && x < (INT64_C(1) << (N - 1)));
}

// Template specializations to get better code for common cases.
template <>
constexpr inline bool isInt<8>(VIntS x) {
    return static_cast<int8_t>(x) == x;
}
template <>
constexpr inline bool isInt<16>(VIntS x) {
    return static_cast<int16_t>(x) == x;
}
template <>
constexpr inline bool isInt<32>(VIntS x) {
    int32_t x2 = static_cast<int32_t>(x);
    return x2 == x;
}

/// Checks if an unsigned integer fits into the given bit width.
///
/// This is written as two functions rather than as simply
///
///   return N >= 64 || X < (UINT64_C(1) << N);
///
/// to keep MSVC from (incorrectly) warning on isUInt<64> that we're shifting
/// left too many places.
template <unsigned N>
constexpr inline std::enable_if_t<(N < 64), bool> isUInt(VInt X) {
    static_assert(N > 0, "isUInt<0> doesn't make sense");
    return X < (UINT64_C(1) << (N));
}
template <unsigned N>
constexpr inline std::enable_if_t<N >= 64, bool> isUInt(VInt) {
    return true;
}

// Template specializations to get better code for common cases.
template <>
constexpr inline bool isUInt<8>(VInt x) {
    return static_cast<uint8_t>(x) == x;
}
template <>
constexpr inline bool isUInt<16>(VInt x) {
    return static_cast<uint16_t>(x) == x;
}
template <>
constexpr inline bool isUInt<32>(VInt x) {
    return static_cast<uint32_t>(x) == x;
}

// Runtime versions of the above
constexpr inline bool isUInt(unsigned N, uint64_t X) {
    return N >= 64 || X < (UINT64_C(1) << (N));
}
constexpr inline bool isInt(unsigned N, VIntS x) {
    return N >= 64 || (-(INT64_C(1) << (N - 1)) <= x && x < (INT64_C(1) << (N - 1)));
}

constexpr bool isPowerOf2(unsigned v) {
    return v && ((v & (v - 1)) == 0);
}

inline unsigned log2Ceil(double v) {
    return static_cast<unsigned>(std::ceil(std::log2(v)));
}

template <typename T>
unsigned firstSetBitIdx(const T& val) {
    for (unsigned i = 0; i < CHAR_BIT * sizeof(T); ++i) {
        if ((val >> i) & 0x1) {
            return i;
        }
    }
    return 0;
}

}  // namespace Ripes
