#ifndef PARSER_H
#define PARSER_H

#include "defines.h"
#include <fstream>
#include <vector>

using namespace std;

class Parser {
public:
  Parser();
  ~Parser();

  int getFileSize() { return m_fileSize; }

public:
  bool init(char *filename);
  void parseFile(memory *text);

private:
  ifstream m_fileStream;
  istreambuf_iterator<char> m_fileIter;
  int m_fileSize;
};

#endif // PARSER_H
