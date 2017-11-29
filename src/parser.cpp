#include "parser.h"

#include <assert.h>
#include <iostream>

Parser::Parser() {
    // generate word parser functors
    m_decodeRInstr = generateWordParser(vector<int>{5, 3, 5, 5, 7});  // from LSB to MSB
    m_decodeIInstr = generateWordParser(vector<int>{5, 3, 5, 12});
    m_decodeSInstr = generateWordParser(vector<int>{5, 3, 5, 5, 7});
    m_decodeBInstr = generateWordParser(vector<int>{1, 4, 3, 5, 5, 6, 1});
    m_decodeUInstr = generateWordParser(vector<int>{5, 20});
    m_decodeJInstr = generateWordParser(vector<int>{5, 8, 1, 10, 1});
}

bool Parser::init(char* filename) {
    // Open binary file
    const string fname = string(filename);
    m_fileStream       = ifstream(fname.c_str(), ios::binary);
    if (!(m_fileStream.good())) {
        return 1;
    }

    // Create filestream iterator
    m_fileIter = istreambuf_iterator<char>(m_fileStream);

    // get file size
    m_fileStream.seekg(0, ios::end);
    m_fileSize = m_fileStream.tellg();
    m_fileStream.clear();
    m_fileStream.seekg(0, ios::beg);
    return 0;
}

Parser::~Parser() {}

void Parser::parseFile(memory* memoryPtr) {
    // Parse the file in 8-bit segments and write to memory map
    int pc = 0;
    while (m_fileIter != istreambuf_iterator<char>()) {
        (*memoryPtr)[pc] = *m_fileIter;
        pc++;
        m_fileIter++;
    }
}

namespace {
uint32_t generateBitmask(int n) {
    // Generate bitmask. There might be a smarter way to do this
    uint32_t mask = 0;
    for (int i = 0; i < n - 1; i++) {
        mask |= 0b1;
        mask <<= 1;
    }
    mask |= 0b1;
    return mask;
}

uint32_t bitcount(int n) {
    int count = 0;
    while (n > 0) {
        count += 1;
        n = n & (n - 1);
    }
    return count;
}
}

decode_functor Parser::generateWordParser(std::vector<int> bitFields) {
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
        parseVector.push_back(std::pair<uint32_t, uint32_t>(field, generateBitmask(field)));
    }

    // Create parse functor
    decode_functor wordParser = [=](uint32_t word) {
        word = word >> 7;  // remove OpCode
        std::vector<uint32_t> parsedWord;
        for (const auto& field : parseVector) {
            parsedWord.insert(parsedWord.begin(), word & field.second);
            word = word >> field.first;
        }
        return parsedWord;
    };

    return wordParser;
}
