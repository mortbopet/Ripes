#pragma once

#include "isainfo.h"

namespace Ripes {

namespace RVISA {

extern const QStringList RegAliases;
extern const QStringList RegNames;
extern const QStringList RegDescs;
enum Opcode { OPIMM = 0b0010011, OPIMM32 = 0b0110011 };

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
