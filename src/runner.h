#ifndef RUNNER_H
#define RUNNER_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "defines.h"
#include "runnercache.h"

class Parser;
typedef std::function<std::vector<uint32_t>(uint32_t)> decode_functor;

class Runner {
public:
  Runner(Parser *parser);
  ~Runner();

  int exec();

  RunnerCache *getRunnerCachePtr() { return &m_cache; }

private:
  instrState execInstruction(Instruction instr);
  void handleError(instrState err) const;

  int m_pc = 0; // program counter

  // Memory - Memory is interfaced through a single function, but allocated
  // seperately. A symbolic pointer is set up to create the virtual index into
  // the total memory
  std::vector<uint32_t> m_reg; // Internal registers
  const uint32_t m_textStart = 0x0;
  std::vector<uint8_t> m_data;
  const uint32_t m_dataStart = 0x10000000;
  std::vector<uint8_t> m_stack;
  const uint32_t m_stackStart = 0x7ffffff0;
  std::vector<uint8_t> m_heap;
  const uint32_t m_heapStart = 0x10008000;

  std::unordered_map<uint32_t, uint8_t> m_memory;

  void memWrite(uint32_t address, uint32_t value, int size);
  uint32_t memRead(uint32_t address);

  int m_memsize;

  Parser *m_parser;

  bool getInstruction(int pc);
  Instruction m_currentInstruction;

  // Instruction execution functions
  instrState execLuiInstr(Instruction instr);
  instrState execAuipcInstr(Instruction instr);
  instrState execJalInstr(Instruction instr);
  instrState execJalrInstr(Instruction instr);
  instrState execBranchInstr(Instruction instr);
  instrState execLoadInstr(Instruction instr);
  instrState execStoreInstr(Instruction instr);
  instrState execOpImmInstr(Instruction instr);
  instrState execOpInstr(Instruction instr);
  instrState execEcallInstr();

  // Instruction decode functions; generated programmatically
  decode_functor generateWordParser(std::vector<int> bitFields);
  decode_functor decodeUInstr;
  decode_functor decodeJInstr;
  decode_functor decodeIInstr;
  decode_functor decodeSInstr;
  decode_functor decodeRInstr;
  decode_functor decodeBInstr;

  // Cache
  RunnerCache m_cache;
};

#endif // RUNNER_H
