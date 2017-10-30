#include "parser.h"

Parser::Parser(char *fileName) {}
Parser::~Parser() {}

bool Parser::parseInstruction(int byteOffset) { return true; }

instrType Parser::getOpType(uint32_t word) { return INVALID; }

Instruction Parser::getInstruction() const { return m_currentInstruction; }
