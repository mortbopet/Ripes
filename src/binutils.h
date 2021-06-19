#pragma once

#include <cstdint>
#include <vector>
#include "limits.h"

#include "VSRTL/interface/vsrtl_binutils.h"

namespace Ripes {

/// Checks if an integer fits into the given bit width.
/// from LLVM MathExtras.h
template <unsigned N>
constexpr inline bool isInt(int64_t x) {
    return N >= 64 || (-(INT64_C(1) << (N - 1)) <= x && x < (INT64_C(1) << (N - 1)));
}

// Template specializations to get better code for common cases.
template <>
constexpr inline bool isInt<8>(int64_t x) {
    return static_cast<int8_t>(x) == x;
}
template <>
constexpr inline bool isInt<16>(int64_t x) {
    return static_cast<int16_t>(x) == x;
}
template <>
constexpr inline bool isInt<32>(int64_t x) {
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
constexpr inline std::enable_if_t<(N < 64), bool> isUInt(uint64_t X) {
    static_assert(N > 0, "isUInt<0> doesn't make sense");
    return X < (UINT64_C(1) << (N));
}
template <unsigned N>
constexpr inline std::enable_if_t<N >= 64, bool> isUInt(uint64_t) {
    return true;
}

// Template specializations to get better code for common cases.
template <>
constexpr inline bool isUInt<8>(uint64_t x) {
    return static_cast<uint8_t>(x) == x;
}
template <>
constexpr inline bool isUInt<16>(uint64_t x) {
    return static_cast<uint16_t>(x) == x;
}
template <>
constexpr inline bool isUInt<32>(uint64_t x) {
    return static_cast<uint32_t>(x) == x;
}

// Runtime versions of the above
constexpr inline bool isUInt(unsigned N, uint64_t X) {
    return X < (UINT64_C(1) << (N));
}
constexpr inline bool isInt(unsigned N, int64_t x) {
    return N >= 64 || (-(INT64_C(1) << (N - 1)) <= x && x < (INT64_C(1) << (N - 1)));
}

constexpr bool isPowerOf2(unsigned v) {
    return v && ((v & (v - 1)) == 0);
}

template <typename T>
unsigned firstSetBitIdx(const T& val) {
    for (unsigned i = 0; i < CHAR_BIT * sizeof(T); i++) {
        if ((val >> i) & 0x1) {
            return i;
        }
    }
    return 0;
}

}  // namespace Ripes
