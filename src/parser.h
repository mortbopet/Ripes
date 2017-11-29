#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>
#include "defines.h"

using namespace std;
typedef std::function<std::vector<uint32_t>(uint32_t)> decode_functor;

class Parser {
public:
    Parser();
    ~Parser();

    int getFileSize() { return m_fileSize; }

    // Const interfaces to intstruction decode lamdas
    std::vector<uint32_t> decodeUInstr(uint32_t instr) const { return m_decodeUInstr(instr); }
    std::vector<uint32_t> decodeJInstr(uint32_t instr) const { return m_decodeJInstr(instr); }
    std::vector<uint32_t> decodeIInstr(uint32_t instr) const { return m_decodeIInstr(instr); }
    std::vector<uint32_t> decodeSInstr(uint32_t instr) const { return m_decodeSInstr(instr); }
    std::vector<uint32_t> decodeRInstr(uint32_t instr) const { return m_decodeRInstr(instr); }
    std::vector<uint32_t> decodeBInstr(uint32_t instr) const { return m_decodeBInstr(instr); }

public:
    bool init(char* filename);
    void parseFile(memory* memoryPtr);

private:
    ifstream m_fileStream;
    istreambuf_iterator<char> m_fileIter;
    int m_fileSize;

    // Instruction decode lambda functions; runtime generated
    decode_functor generateWordParser(std::vector<int> bitFields);
    decode_functor m_decodeUInstr;
    decode_functor m_decodeJInstr;
    decode_functor m_decodeIInstr;
    decode_functor m_decodeSInstr;
    decode_functor m_decodeRInstr;
    decode_functor m_decodeBInstr;
};

#endif  // PARSER_H
