#include "parser.h"

#include <assert.h>
#include <iostream>

#include "binutils.h"

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
    m_fileStream = ifstream(fname.c_str(), ios::binary);
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

QString Parser::genStringRepr(uint32_t instr) const {
    switch (instr & 0x7f) {
        case LUI:
            return generateLuiString(instr);
        case AUIPC:
            return generateAuipcString(instr);
        case JAL:
            return generateJalString(instr);
        case JALR:
            return generateJalrString(instr);
        case BRANCH:
            return generateBranchString(instr);
        case LOAD:
            return generateLoadString(instr);
        case STORE:
            return generateStoreString(instr);
        case OP_IMM:
            return generateOpImmString(instr);
        case OP:
            return generateOpInstrString(instr);
        case ECALL:
            return generateEcallString(instr);
        default:
            return QString("Invalid instruction");
            break;
    }
}
QString Parser::generateEcallString(uint32_t instr) const {
    return QString();
}

QString Parser::generateOpInstrString(uint32_t instr) const {
    return QString();
}

QString Parser::generateOpImmString(uint32_t instr) const {
    return QString();
}

QString Parser::generateStoreString(uint32_t instr) const {
    return QString();
}

QString Parser::generateLoadString(uint32_t instr) const {
    return QString();
}

QString Parser::generateBranchString(uint32_t instr) const {
    std::vector<uint32_t> fields = decodeBInstr(instr);
    int offset = signextend<int32_t, 13>((fields[0] << 12) | (fields[1] << 5) | (fields[5] << 1) | (fields[6] << 11));
    switch (fields[4]) {
        case 0b000:  // BEQ
            return QString("beq x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b001:  // BNE
            return QString("bne x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b100:  // BLT - signed comparison
            return QString("blt x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b101:  // BGE - signed comparison
            return QString("bge x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b110:  // BLTU
            return QString("bltu x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        case 0b111:  // BGEU
            return QString("bgeu x%1 x%2 %3").arg(fields[3]).arg(fields[2]).arg(offset);
        default:
            return QString();
    }
}

QString Parser::generateJalrString(uint32_t instr) const {
    return QString();
}

QString Parser::generateLuiString(uint32_t instr) const {
    return QString();
}

QString Parser::generateAuipcString(uint32_t instr) const {
    return QString();
}

QString Parser::generateJalString(uint32_t instr) const {
    return QString();
}
