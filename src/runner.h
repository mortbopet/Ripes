#ifndef RUNNER_H
#define RUNNER_H

#include <functional>
#include <memory>
#include <vector>

#include "defines.h"

class Parser;
typedef std::function<std::vector<uint32_t>(uint32_t)> decode_functor;

class Runner {
public:
  Runner(Parser *parser);
  ~Runner();

  int exec();

private:
  instrState execInstruction(Instruction instr);
  void handleError(instrState err) const;

  int m_pc = 0;                // program counter
  std::vector<uint32_t> m_reg; // Internal registers
  uint8_t *m_mem;              // Stack/Heap
  std::vector<uint8_t> m_text; // text segment
  int m_textSize;
  uint8_t *m_data; // data segment

  int m_memsize;

  Parser *m_parser;

  bool getInstruction(int pc);
  Instruction m_currentInstruction;

  // Instruction execution functions
  instrState execLuiInstr(Instruction instr);
  instrState execJalInstr(Instruction instr);
  instrState execJalrInstr(Instruction instr);
  instrState execBranchInstr(Instruction instr);
  instrState execLoadInstr(Instruction instr);
  instrState execStoreInstr(Instruction instr);
  instrState execOpImmInstr(Instruction instr);
  instrState execOpInstr(Instruction instr);

  // Instruction decode functions; generated programmatically
  decode_functor generateWordParser(std::vector<int> bitFields);
  decode_functor decodeUInstr;
  decode_functor decodeJInstr;
  decode_functor decodeIInstr;
  decode_functor decodeSInstr;
  decode_functor decodeRInstr;
  decode_functor decodeBInstr;
};

#endif // RUNNER_H
