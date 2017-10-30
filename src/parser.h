#ifndef PARSER_H
#define PARSER_H

#include "defines.h"

class Parser {
public:
  Parser(char *fileName);
  ~Parser();

  bool parseInstruction(int byteOffset = 4);
  uint32_t readWord();
  instrType getOpType(uint32_t word);

  Instruction getInstruction() const;

private:
  Instruction m_currentInstruction;
  // FILE *filePtr;
};

#endif // PARSER_H
