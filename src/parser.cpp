#include "parser.h"

Parser::Parser(char *fileName) {}
Parser::~Parser() {}

bool Parser::parseInstruction(int pc) {
  // Given a program counter, parse instruction from binary file. Return false
  // if instruction was not able to be parsed (or EOF)

  // m_currentInstruction = ...

  return true;
}

instrType Parser::getOpType(uint32_t word) { return INVALID; }

Instruction Parser::getInstruction() const { return m_currentInstruction; }
