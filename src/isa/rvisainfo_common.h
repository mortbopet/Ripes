#pragma once

#include "isainfo.h"

namespace Ripes {

namespace RVISA {

extern const QStringList RegAliases;
extern const QStringList RegNames;
extern const QStringList RegDescs;
enum Opcode {
    LUI = 0b0110111,
    JAL = 0b1101111,
    JALR = 0b1100111,
    BRANCH = 0b1100011,
    LOAD = 0b0000011,
    STORE = 0b0100011,
    OPIMM = 0b0010011,
    OP = 0b0110011,
    OPIMM32 = 0b0011011,
    OP32 = 0b0111011,
    ECALL = 0b1110011,
    AUIPC = 0b0010111,
    INVALID = 0b0
};

}  // namespace RVISA

namespace RVABI {
// RISC-V ELF info
// Elf flag masks
enum RVElfFlags { RVC = 0b1, FloatABI = 0b110, RVE = 0b1000, TSO = 0b10000 };
extern const std::map<RVElfFlags, QString> ELFFlagStrings;

enum SysCall {
    None = 0,
    PrintInt = 1,
    PrintFloat = 2,
    PrintStr = 4,
    Exit = 10,
    PrintChar = 11,
    GetCWD = 17,
    TimeMs = 30,
    Cycles = 31,
    PrintIntHex = 34,
    PrintIntBinary = 35,
    PrintIntUnsigned = 36,
    Close = 57,
    LSeek = 62,
    Read = 63,
    Write = 64,
    FStat = 80,
    Exit2 = 93,
    brk = 214,
    Open = 1024
};

}  // namespace RVABI
}  // namespace Ripes
