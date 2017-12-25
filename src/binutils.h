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

constexpr uint32_t generateBitmask(int n) {
    // Generate bitmask. There might be a smarter way to do this
    uint32_t mask = 0;
    for (int i = 0; i < n - 1; i++) {
        mask |= 0b1;
        mask <<= 1;
    }
    mask |= 0b1;
    return mask;
}

constexpr uint32_t bitcount(int n) {
    int count = 0;
    while (n > 0) {
        count += 1;
        n = n & (n - 1);
    }
    return count;
}

inline uint32_t accBVec(std::vector<bool> v) {
    uint32_t r = 0;
    for (uint32_t i = 0; i < v.size(); i++) {
        r |= v[i] << i;
    }
    return r;
}

inline void buildVec(std::vector<bool>& v, uint32_t n) {
    for (uint32_t i = 0; i < v.size(); i++) {
        v[i] = n & 0b1;
        n >>= 1;
    }
}
