#include "parser.h"

Parser::Parser(char *fileName) {}

instrType Parser::getOpType(uint32_t word) { return INVALID; }

Instruction Parser::getInstruction() const { return m_currentInstruction; }
