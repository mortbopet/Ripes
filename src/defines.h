#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>

enum instrType {
  LUI = 0b0110111,
  JAL = 0b1101111,
  JALR = 0b1100111,
  BRANCH = 0b1100011,
  LOAD = 0b0000011,
  STORE = 0b0100011,
  OP_IMM = 0b0010011,
  OP = 0b0110011,
  INVALID = 0b0
};

enum error { INV_IMM, ALLGOOD };

typedef struct {
  instrType type = INVALID;
  uint32_t word = 0;
} Instruction;

#endif // DEFINES_H
