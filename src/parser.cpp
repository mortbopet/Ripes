#include "parser.h"
#include <fstream>

Parser::Parser(char *fileName) {}
Parser::~Parser() {}

uint32_t Parser::readWord() { return 0; }

bool Parser::parseInstruction(int pc) {
  // Given a program counter, parse instruction from binary file. Return false
  // if instruction was not able to be parsed (or EOF)

  // m_currentInstruction = ...

  // For first time, ceate object. Check object, and report error if
  // incorrect file or path
  /*
if (!binaryFile) {
  ifstream binaryFile(*fileName, ios::binary);
}
if (!binaryFile.good()) {
  return false;
}

// Read file with respect to pc, and put into uint32. If not possible, return
// false.
if (binaryFile.read(readWord(), pc)) {
  binaryFile.close();
  return true;
} else {
  return false;
}
*/
  return false;
}

instrType Parser::getOpType(uint32_t word) { return INVALID; }

Instruction Parser::getInstruction() const { return m_currentInstruction; }
