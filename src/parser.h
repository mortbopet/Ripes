#ifndef PARSER_H
#define PARSER_H

#include <cstdint>

class Parser {
public:
  Parser(char *fileName);
  ~Parser();

  int getInstruction(int byteOffset = 4);
  uint32_t readWord();
  int decodeInstruction(uint32_t word);

private:
  // FILE *filePtr;
};

#endif // PARSER_H
