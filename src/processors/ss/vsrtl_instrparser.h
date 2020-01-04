#pragma once

#include <functional>
#include "VSRTL/core/vsrtl_binutils.h"

namespace vsrtl {
using namespace core;
typedef std::function<std::vector<uint32_t>(uint32_t)> decode_functor;
decode_functor generateInstrParser(std::vector<int> bitFields) {
    // Generates functors that can decode a binary number based on the input
    // vector which is supplied upon generation

    // Assert that total bitField size is (32-7)=25-bit. Subtract 7 for op-code
    int tot = 0;
    for (const auto& field : bitFields) {
        tot += field;
    }
    assert(tot == 25 && "Requested word parsing format is not 32-bit in length");

    // Generate vector of <fieldsize,bitmask>
    std::vector<std::pair<uint32_t, uint32_t>> parseVector;

    // Generate bit masks and fill parse vector
    for (const auto& field : bitFields) {
        parseVector.emplace_back(field, generateBitmask(field));
    }

    // Create parse functor
    decode_functor instrParser = [=](uint32_t word) {
        word = word >> 7;  // remove OpCode
        std::vector<uint32_t> parsedWord;
        for (const auto& field : parseVector) {
            parsedWord.insert(parsedWord.begin(), word & field.second);
            word = word >> field.first;
        }
        return parsedWord;
    };

    return instrParser;
}

}  // namespace vsrtl
