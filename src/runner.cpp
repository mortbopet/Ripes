#include "runner.h"

#include "parser.h"

Runner::Runner(Parser *parser) { m_parser = parser; }

Runner::~Runner() {}

int Runner::exec() {
  // Main simulator loop
  error err;
  while (m_parser->parseInstruction()) {
    if ((err = execInstruction(m_parser->getInstruction())) != error::ALLGOOD) {
      handleError(err);
    }
  }
  return 0;
}

error Runner::execInstruction(Instruction instr) { return error::ALLGOOD; }

void Runner::handleError(error err) {}
