#ifndef RUNNER_H
#define RUNNER_H

#include <memory>
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
  void handleError(error err) const;

  int m_pc = 0;       // program counter
  uint32_t m_reg[32]; // Internal registers
  uint8_t *m_mem;     // Stack/Heap
  uint8_t *m_text;    // text segment
  int m_textSize;
  uint8_t *m_data; // data segment

  int m_memsize;

  Parser *m_parser;

  bool getInstruction(int pc);
  Instruction m_currentInstruction;

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
  std::vector<uint32_t> decodeUInstr(Instruction instr) const;
  std::vector<uint32_t> decodeJInstr(Instruction instr) const;
  std::vector<uint32_t> decodeIInstr(Instruction instr) const;
  std::vector<uint32_t> decodeSInstr(Instruction instr) const;
  std::vector<uint32_t> decodeRInstr(Instruction instr) const;
  std::vector<uint32_t> decodeBInstr(Instruction instr) const;
};

#endif // RUNNER_H
