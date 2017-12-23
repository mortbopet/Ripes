#ifndef DEFINES_H
#define DEFINES_H

#include <QList>
#include <QPair>
#include <QString>
#include <cstdint>
#include <map>
#include <vector>
#include "unordered_map"

#define WORDSIZE 32

enum displayTypeN { Hex = 1, Binary = 2, Decimal = 3, Unsigned = 4, ASCII = 5 };
Q_DECLARE_METATYPE(displayTypeN)

typedef struct {
    // Initialize to UINT32_MAX. An instruction at PC = 0 should not be defaulted as being inside a pipeline -
    // therefore, the maximum uint32 value is chosen
    uint32_t IF = UINT32_MAX;
    uint32_t ID = UINT32_MAX;
    uint32_t EX = UINT32_MAX;
    uint32_t MEM = UINT32_MAX;
    uint32_t WB = UINT32_MAX;
} StagePCS;

enum Colors {
    // Berkeley primary color palette
    BerkeleyBlue = 0x003262,
    FoundersRock = 0x3B7EA1,
    CaliforniaGold = 0xFDB515,
    Medalist = 0xC4820E
};

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
}  // namespace

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

enum class cacheLevel { L1, L2, L3 };

enum class cacheType { DM, SA, FA };

typedef std::unordered_map<uint32_t, uint8_t> memory;

enum class runnerState { ERR_BFUNCT3, ERR_NULLLOAD, EXEC_ERR, SUCCESS, DONE, ERR_ECALL, BREAKPOINT };

const static QStringList ABInames = QStringList() << "zero"
                                                  << "ra"
                                                  << "sp"
                                                  << "gp"
                                                  << "tp"
                                                  << "t0"
                                                  << "t1"
                                                  << "t2"
                                                  << "s0/fp"
                                                  << "s1"
                                                  << "a0"
                                                  << "a1"
                                                  << "a2"
                                                  << "a3"
                                                  << "a4"
                                                  << "a5"
                                                  << "a6"
                                                  << "a7"
                                                  << "s2"
                                                  << "s3"
                                                  << "s4"
                                                  << "s5"
                                                  << "s6"
                                                  << "s7"
                                                  << "s8"
                                                  << "s9"
                                                  << "s10"
                                                  << "s11"
                                                  << "t3"
                                                  << "t4"
                                                  << "t5"
                                                  << "t6";

const static std::map<int, QString> cacheSizes = {{32, QString("32 Bytes")},   {64, QString("64 Bytes")},
                                                  {128, QString("128 Bytes")}, {256, {QString("256 Bytes")}},
                                                  {512, QString("512 Bytes")}, {1024, QString("1024 Bytes")}};

const static std::map<QString, cacheType> cacheTypes = {{QString("Direct mapped"), cacheType::DM},
                                                        {QString("Set associative"), cacheType::SA},
                                                        {QString("Fully associative"), cacheType::FA}};

typedef struct {
    instrType type = INVALID;
    uint32_t word = 0;
} Instruction;

#endif  // DEFINES_H
