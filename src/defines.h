#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>
#include <vector>

enum instrType {
  LUI = 0b0110111,
  JAL = 0b1101111,
  JALR = 0b1100111,
  BRANCH = 0b1100011,
  LOAD = 0b0000011,
  STORE = 0b0100011,
  OP_IMM = 0b0010011,
  OP = 0b0110011,
  ECALL = 0b1110011,
  AUIPC = 0b0010111,
  INVALID = 0b0
};

typedef std::vector<uint8_t> memory;

enum instrState {
  ERR_BFUNCT3,
  ERR_NULLLOAD,
  EXEC_ERR,
  SUCCESS = 0,
  DONE,
  ERR_ECALL
};

typedef struct {
  instrType type = INVALID;
  uint32_t word = 0;
} Instruction;

#endif // DEFINES_H
