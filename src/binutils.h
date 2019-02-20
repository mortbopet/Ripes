#pragma once

#include <cstdint>
#include <vector>

// Sign extension of arbitrary bitfield size.
// Courtesy of http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
template <typename T, unsigned B>
inline T signextend(const T x) {
    struct {
        T x : B;
    } s;
    return s.x = x;
}

/// Checks if an integer fits into the given bit width.
/// from LLVM MathExtras.h
template <unsigned N>
constexpr inline bool isInt(int64_t x) {
    return N >= 64 || (-(INT64_C(1) << (N - 1)) <= x && x < (INT64_C(1) << (N - 1)));
}

constexpr uint32_t generateBitmask(int n) {
    return static_cast<uint32_t>((1 << n) - 1);
}

constexpr uint32_t bitcount(int n) {
    int count = 0;
    while (n > 0) {
        count += 1;
        n = n & (n - 1);
    }
    return count;
}

uint32_t accBVec(const std::vector<bool>& v);
void buildVec(std::vector<bool>& v, uint32_t n);
