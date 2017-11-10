#ifndef DEFINES_H
#define DEFINES_H

#include "unordered_map"
#include <QList>
#include <QPair>
#include <QString>
#include <cstdint>
#include <map>
#include <vector>

enum displayTypeN { Hex = 1, Binary = 2, Decimal = 3, Unsigned = 4, ASCII = 5 };
Q_DECLARE_METATYPE(displayTypeN)

namespace {
static QMap<QString, displayTypeN> initDisplayTypes() {
  QMap<QString, displayTypeN> types;
  types.insert("Hex", displayTypeN::Hex);
  types.insert("Binary", displayTypeN::Binary);
  types.insert("Decimal", displayTypeN::Decimal);
  types.insert("Unsigned", displayTypeN::Unsigned);
  types.insert("ASCII", displayTypeN::ASCII);
  return types;
}
}

const static QMap<QString, displayTypeN> displayTypes = initDisplayTypes();

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

typedef std::unordered_map<uint32_t, uint8_t> memory;

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
