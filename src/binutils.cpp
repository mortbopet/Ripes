#include "binutils.h"

uint32_t accBVec(const std::vector<bool>& v) {
    uint32_t r = 0;
    for (uint32_t i = 0; i < v.size(); i++) {
        r |= v.at(i) << i;
    }
    return r;
}

void buildVec(std::vector<bool>& v, uint32_t n) {
    for (auto && i : v) {
        i = n & 0b1;
        n >>= 1;
    }
}
