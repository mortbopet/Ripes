#ifndef DEFINES_H
#define DEFINES_H

#include <QString>
#include <cstdint>
#include <map>
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

enum cacheLevel { L1 = 0, L2 = 1, L3 = 2 };

enum cacheType { DM, SA, FA };

typedef std::vector<uint8_t> memory;

enum instrState {
  ERR_BFUNCT3,
  ERR_NULLLOAD,
  EXEC_ERR,
  SUCCESS,
  DONE,
  ERR_ECALL
};

const static std::map<int, QString> cacheSizes = {
    {32, QString("32 Bytes")},   {64, QString("64 Bytes")},
    {128, QString("128 Bytes")}, {256, {QString("256 Bytes")}},
    {512, QString("512 Bytes")}, {1024, QString("1024 Bytes")}};

const static std::map<QString, int> cacheTypes = {
    {QString("Direct mapped"), DM},
    {QString("Set associative"), SA},
    {QString("Fully associative"), FA}};

typedef struct {
  instrType type = INVALID;
  uint32_t word = 0;
} Instruction;

#endif // DEFINES_H
