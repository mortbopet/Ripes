#ifndef RUNNER_H
#define RUNNER_H

#include "defines.h"

class Runner {
public:
  Runner();
  ~Runner();

private:
  int m_pc;           // program counter
  uint32_t m_reg[32]; // Internal registers
  uint32_t *m_mem;    // Data memory
};

#endif // RUNNER_H
