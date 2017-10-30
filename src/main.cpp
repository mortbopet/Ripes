#include <iostream>

#include "parser.h"
#include "runner.h"

using namespace std;

int main(int argc, char **argv) {
  /*
if (argc != 2) {
  // insert error msg
  return 1;
}
*/

  uint8_t a[4] = {1, 2, 3, 4};

  Parser parser(argv[1]);
  Runner runner(&parser);

  // execute runner
  return runner.exec();
}
