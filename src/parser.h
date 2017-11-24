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

public:
    bool init(char* filename);
    void parseFile(memory* memoryPtr);

private:
    ifstream m_fileStream;
    istreambuf_iterator<char> m_fileIter;
    int m_fileSize;

    // Instruction decode functions; runtime generated
    decode_functor generateWordParser(std::vector<int> bitFields);
    decode_functor decodeUInstr;
    decode_functor decodeJInstr;
    decode_functor decodeIInstr;
    decode_functor decodeSInstr;
    decode_functor decodeRInstr;
    decode_functor decodeBInstr;
};

#endif  // PARSER_H
