#pragma once

#include "isainfo.h"
#include <QDebug>

namespace Ripes {

template <unsigned XLEN>
constexpr ISA XLenToMIPSISA() {
  static_assert(XLEN == 32, "Only supports 32-bit variants");
  return ISA::MIPS32I;
}

namespace MIPSISA {

extern const QStringList RegAliases;
extern const QStringList RegNames;
extern const QStringList RegDescs;
enum Opcode {
  ADDI = 0b001000,
  ADDIU = 0b001001,
  ANDI = 0b001100,
  RTYPE = 0b000000,
  ORI = 0b001101,
  XORI = 0b001110,
  LHI = 0b011001,
  LLO = 0b011000,
  SLTI = 0b001010,
  SLTIU = 0b001011,
  BEQ = 0b000100,
  BGEZ = 0b000001,
  BGTZ = 0b000111,
  BLEZ = 0b000110,
  BLTZ = 0b000001,
  BNE = 0b000101,
  J = 0b000010,
  JAL = 0b000011,
  LB = 0b100000,
  LBU = 0b100100,
  LH = 0b100001,
  LHU = 0b100101,
  LUI = 0b001111,
  LW = 0b100011,
  LWC1 = 0b110001,
  SB = 0b101000,
  SH = 0b101001,
  SW = 0b101011,
  SWC1 = 0b111001,
  TRAP = 0b011010
};

enum Function {
  ADD = 0b100000,
  ADDU = 0b100001,
  AND = 0b100100,
  DIV = 0b011010,
  DIVU = 0b011011,
  MULT = 0b011000,
  MULTU = 0b011001,
  NOR = 0b100111,
  OR = 0b100101,
  SLL = 0b000000,
  SLLV = 0b000100,
  SRA = 0b000011,
  SRAV = 0b000111,
  SRL = 0b000010,
  SRLV = 0b000110,
  SUB = 0b100010,
  SUBU = 0b100011,
  XOR = 0b100110,
  SLT = 0b101010,
  SLTU = 0b101001,
  JALR = 0b001001,
  JR = 0b001000,
  MFHI = 0b010000,
  MFLO = 0b010010,
  MTHI = 0b010001,
  MTLO = 0b010011,
  BREAK = 0b001101,
  SYSCALL = 0b001100
};

} // namespace MIPSISA

namespace MIPSABI {
// MIPS ELF info
// Elf flag masks
enum MIPSElfFlags { NOREORDER = 0b1, PIC = 0b10, CPIC = 0b100, ARCH = 0b0 };
extern const std::map<MIPSElfFlags, QString> ELFFlagStrings;

enum SysCall {
  None = 0,
  PrintInt = 1,
  PrintFloat = 2,
  PrintDouble = 3,
  PrintStr = 4,
  ReadInt = 5,
  ReadFloat = 6,
  ReadDouble = 7,
  ReadString = 8,
  Sbrk = 9,
  Exit = 10,
  PrintChar = 11,
  ReadChar = 12,
  Open = 13,
  Read = 14,
  Write = 15,
  Close = 16,
  Exit2 = 17,
  Time = 30,
  MIDIout = 31,
  Sleep = 32,
  MIDIoutSynchronous = 33,
  PrintIntHex = 34,
  PrintIntBinary = 35,
  PrintIntUnsigned = 36,
  SetSeed = 40,
  RandomInt = 41,
  RandomIntRange = 42,
  RandomFloat = 43,
  RandomDouble = 44,
  ConfirmDialog = 50,
  InputDialogInt = 51,
  InputDialogFloat = 52,
  InputDialogDouble = 53,
  InutDialogString = 54,
  MessageDialog = 55,
  MessageDialogInt = 56,
  MessageDialogFloat = 57,
  MessageDialogDouble = 58,
  MessageDialogString = 59

};

} // namespace MIPSABI

class MIPSISAInfoBase : public ISAInfoBase {
public:
  unsigned int regCnt() const override { return 34; }
  QString regName(unsigned i) const override {
    return (MIPSISA::RegNames.size() > static_cast<int>(i)
                ? MIPSISA::RegNames.at(static_cast<int>(i))
                : QString());
  }
  QString regAlias(unsigned i) const override {
    return MIPSISA::RegAliases.size() > static_cast<int>(i)
               ? MIPSISA::RegAliases.at(static_cast<int>(i))
               : QString();
  }
  QString regInfo(unsigned i) const override {
    return MIPSISA::RegDescs.size() > static_cast<int>(i)
               ? MIPSISA::RegDescs.at(static_cast<int>(i))
               : QString();
  }
  QString name() const override { return CCmarch().toUpper(); }
  bool regIsReadOnly(unsigned i) const override { return i == 0; }
  int spReg() const override { return 29; }
  int gpReg() const override { return 28; }
  int syscallReg() const override { return 2; }
  unsigned instrBits() const override { return 32; }
  unsigned elfMachineId() const override { return EM_MIPS; }
  unsigned int regNumber(const QString &reg, bool &success) const override {
    QString regRes = reg;
    success = true;
    if (reg[0] != '$') {
      success = false;
      return 0;
    }

    QString regNoDollar = regRes.remove('$');

    if (MIPSISA::RegNames.count(reg) != 0) {
      regRes.remove('$');
      return regRes.toInt(&success, 10);
    } else if (int idx = MIPSISA::RegAliases.indexOf(regNoDollar); idx != -1) {
      return idx;
    }
    success = false;
    return 0;
  }
  virtual int syscallArgReg(unsigned argIdx) const override {
    assert(argIdx < 2 && "MIPS only implements argument registers a0-a7");
    return argIdx + 4;
  }

  QString elfSupportsFlags(unsigned flags) const override {
    if (flags == 0)
      return QString();
    for (const auto &flag : MIPSABI::ELFFlagStrings)
      flags &= ~flag.first;

    if (flags != 0) {
      return "ELF flag '0b" + QString::number(flags, 2) + "' unsupported";
    }
    return QString();
  }

  const QStringList &supportedExtensions() const override {
    return m_supportedExtensions;
  }
  const QStringList &enabledExtensions() const override {
    return m_enabledExtensions;
  }
  QString extensionDescription(const QString &ext) const override { return ""; }

protected:
  QStringList m_enabledExtensions;
  QStringList m_supportedExtensions = {""};
};

} // namespace Ripes
