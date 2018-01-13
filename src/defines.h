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
#define STACKSTART 0x7ffffff0
#define DATASTART 0x10000000

enum class Stage { IF, ID, EX, MEM, WB };

enum displayTypeN { Hex, Binary, Decimal, Unsigned, ASCII };
Q_DECLARE_METATYPE(displayTypeN)

class StagePCS {
public:
    typedef struct {
        uint32_t pc;
        bool initialized;        // If false, no text will be written above a pipeline stage when pipeline is reset
        uint32_t invalidReason;  // 1: a stage has been flushed because of branch taken
                                 // 2: A stage is stalled because of hazards
    } PC;
    StagePCS() {}
    void reset() {
        IF = PC{0, false, false};
        ID = PC{0, false, false};
        EX = PC{0, false, false};
        MEM = PC{0, false, false};
        WB = PC{0, false, false};
    }
    PC IF;
    PC ID;
    PC EX;
    PC MEM;
    PC WB;
};

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

const static QStringList ASMDirectives = QStringList() << "text";

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
                                                  << "s0"
                                                  << "a0"
                                                  << "a1"
                                                  << "a2"
                                                  << "a3"
                                                  << "a4"
                                                  << "a5"
                                                  << "a6"
                                                  << "a7"
                                                  << "s1"
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

const static QStringList RegNames = QStringList() << "x0"
                                                  << "x1"
                                                  << "x2"
                                                  << "x3"
                                                  << "x4"
                                                  << "x5"
                                                  << "x6"
                                                  << "x7"
                                                  << "x8"
                                                  << "x9"
                                                  << "x10"
                                                  << "x11"
                                                  << "x12"
                                                  << "x13"
                                                  << "x14"
                                                  << "x15"
                                                  << "x16"
                                                  << "x17"
                                                  << "x18"
                                                  << "x19"
                                                  << "x20"
                                                  << "x21"
                                                  << "x22"
                                                  << "x23"
                                                  << "x24"
                                                  << "x25"
                                                  << "x26"
                                                  << "x27"
                                                  << "x28"
                                                  << "x29"
                                                  << "x30"
                                                  << "x31";

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
