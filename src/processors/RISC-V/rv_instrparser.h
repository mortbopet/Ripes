#pragma once

#include <assert.h>
#include <functional>
#include <numeric>
#include "../../binutils.h"

namespace Ripes {

template <typename T>
using decode_functor = std::function<std::vector<T>(T)>;

template <typename T>
decode_functor<T> generateInstrParser(const std::vector<int>& bitFields) {
    constexpr int size_bits = sizeof(T) * CHAR_BIT;
    static_assert(isPowerOf2(size_bits) && size_bits >= 32, "Invalid word size parameter");
    // Generates functors that can decode a binary number based on the input
    // vector which is supplied upon generation
    assert(std::accumulate(bitFields.begin(), bitFields.end(), 0) == size_bits &&
           "Requested word parsing format is not T-bits in length");

    // Generate vector of <fieldsize,bitmask>
    std::vector<std::pair<uint32_t, uint32_t>> parseVector;

    // Generate bit masks and fill parse vector
    for (const auto& field : bitFields) {
        parseVector.emplace_back(field, vsrtl::generateBitmask(field));
    }

    // Create parse functor
    decode_functor<T> instrParser = [=](T word) {
        std::vector<T> parsedWord;
        for (const auto& field : parseVector) {
            parsedWord.insert(parsedWord.begin(), word & field.second);
            word = word >> field.first;
        }
        return parsedWord;
    };

    return instrParser;
}

}  // namespace Ripes
