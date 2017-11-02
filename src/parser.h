#ifndef PARSER_H
#define PARSER_H

#include "defines.h"
#include <fstream>

using namespace std;

class Parser {
public:
  Parser();
  ~Parser();

  int getFileSize();

public:
  bool init(char *filename);
  void parseFile(uint8_t *textPtr);

private:
  ifstream m_fileStream;
  istreambuf_iterator<char> m_fileIter;
  int m_fileSize;
};

#endif // PARSER_H
