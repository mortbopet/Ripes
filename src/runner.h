#ifndef RUNNER_H
#define RUNNER_H

#include <vector>

#include "defines.h"

class Parser;

class Runner {
public:
  Runner(Parser *parser);
  ~Runner();

  int exec();

private:
  error execInstruction(Instruction instr);
  void handleError(error err);

  int m_pc;           // program counter
  uint32_t m_reg[32]; // Internal registers
  uint32_t *m_mem;    // Data memory
  int m_memsize;

  Parser *m_parser;

  // Instruction execution functions
  error execLuiInstr(Instruction instr);
  error execJalInstr(Instruction instr);
  error execJalrInstr(Instruction instr);
  error execBranchInstr(Instruction instr);
  error execLoadInstr(Instruction instr);
  error execStoreInstr(Instruction instr);
  error execOpImmInstr(Instruction instr);
  error execOpInstr(Instruction instr);

  // Instruction decode functions
  std::vector<uint32_t> decodeUInstr(Instruction instr);
  std::vector<uint32_t> decodeJInstr(Instruction instr);
  std::vector<uint32_t> decodeIInstr(Instruction instr);
  std::vector<uint32_t> decodeSInstr(Instruction instr);
  std::vector<uint32_t> decodeRInstr(Instruction instr);
  std::vector<uint32_t> decodeBInstr(Instruction instr);
};

#endif // RUNNER_H
