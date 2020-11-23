#pragma once

#include <cstdint>
#include <vector>
#include "limits.h"

namespace Ripes {

// Sign extension of arbitrary bitfield size.
// Courtesy of http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
template <typename T, unsigned B>
inline T signextend(const T x) {
    struct {
        T x : B;
    } s;
    return s.x = x;
}

// Runtime signextension
template <typename T>
inline T signextend(const T x, unsigned B) {
    int const m = CHAR_BIT * sizeof(T) - B;
    return (x << m) >> m;
}

/// Checks if an integer fits into the given bit width.
/// from LLVM MathExtras.h
template <unsigned N>
constexpr inline bool isInt(int64_t x) {
    return N >= 64 || (-(INT64_C(1) << (N - 1)) <= x && x < (INT64_C(1) << (N - 1)));
}

/// Checks if an unsigned integer fits into the given bit width.
/// from LLVM MathExtras.h
template <unsigned N>
constexpr inline std::enable_if_t<(N < 64), bool> isUInt(uint64_t X) {
    static_assert(N > 0, "isUInt<0> doesn't make sense");
    return X < (UINT64_C(1) << (N));
}

// Runtime versions of the above
constexpr inline bool isUInt(unsigned N, uint64_t X) {
    return X < (UINT64_C(1) << (N));
}
constexpr inline bool isInt(unsigned N, int64_t x) {
    return N >= 64 || (-(INT64_C(1) << (N - 1)) <= x && x < (INT64_C(1) << (N - 1)));
}

constexpr uint32_t generateBitmask(int n) {
    return static_cast<uint32_t>((1 << n) - 1);
}

constexpr uint32_t bitcount(unsigned n) {
    int count = 0;
    while (n > 0) {
        count += 1;
        n = n & (n - 1);
    }
    return count;
}

constexpr bool isPowerOf2(unsigned v) {
    return v && ((v & (v - 1)) == 0);
}

uint32_t accBVec(const std::vector<bool>& v);
void buildVec(std::vector<bool>& v, uint32_t n);

constexpr inline unsigned floorlog2(unsigned x) {
    return x == 1 ? 0 : 1 + floorlog2(x >> 1);
}

constexpr inline unsigned ceillog2(unsigned x) {
    return x == 1 || x == 0 ? 1 : floorlog2(x - 1) + 1;
}

}  // namespace Ripes
