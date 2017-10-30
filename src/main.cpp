#include <iostream>

#include "parser.h"
#include "runner.h"

using namespace std;

int main(/*argv, argc*/) {

  cout << "Hello World!" << endl;

  char *ex;
  Parser parser(ex);
  Runner runner(&parser);

  return runner.exec();
}
