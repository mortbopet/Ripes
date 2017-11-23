#include "parser.h"

#include <iostream>

Parser::Parser() {}

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
