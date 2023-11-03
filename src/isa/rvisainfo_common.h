#pragma once

#include "instruction.h"
#include "isainfo.h"
#include "pseudoinstruction.h"

namespace Ripes {

template <unsigned XLEN>
constexpr ISA XLenToRVISA() {
  static_assert(XLEN == 32 || XLEN == 64,
                "Only supports 32- and 64-bit variants");
  if constexpr (XLEN == 32) {
    return ISA::RV32I;
  } else {
    return ISA::RV64I;
  }
}

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

} // namespace RVABI

namespace RVISA {
extern const QStringList GPRRegAliases;
extern const QStringList GPRRegNames;
extern const QStringList GPRRegDescs;

constexpr unsigned INSTR_BITS = 32;

template <typename InstrImpl>
struct RV_Instruction : public Instruction<InstrImpl> {
  constexpr static unsigned InstrBits() { return INSTR_BITS; }
};

/// Defines information about the general RISC-V register file.
struct RV_GPRInfo : public RegInfoBase {
  constexpr static RegisterFileType RegFileType() {
    return RegisterFileType::GPR;
  }
  constexpr static unsigned int RegCnt() { return 32; }
  static QString RegName(unsigned i) {
    return RVISA::GPRRegNames.size() > static_cast<int>(i)
               ? RVISA::GPRRegNames.at(static_cast<int>(i))
               : QString();
  }
  static QString RegAlias(unsigned i) {
    return RVISA::GPRRegAliases.size() > static_cast<int>(i)
               ? RVISA::GPRRegAliases.at(static_cast<int>(i))
               : QString();
  }
  static QString RegInfo(unsigned i) {
    return RVISA::GPRRegDescs.size() > static_cast<int>(i)
               ? RVISA::GPRRegDescs.at(static_cast<int>(i))
               : QString();
  }
  static bool RegIsReadOnly(unsigned i) { return i == 0; }
  static unsigned int RegNumber(const QString &reg, bool &success) {
    QString regRes = reg;
    success = true;
    if (reg[0] == 'x' && (RVISA::GPRRegNames.count(reg) != 0)) {
      regRes.remove('x');
      return regRes.toInt(&success, 10);
    } else if (RVISA::GPRRegAliases.contains(reg)) {
      return RVISA::GPRRegAliases.indexOf(reg);
    }
    success = false;
    return 0;
  }

  RegisterFileType regFileType() const override { return RegFileType(); }
  unsigned int regCnt() const override { return RegCnt(); }
  QString regName(unsigned i) const override { return RegName(i); }
  QString regAlias(unsigned i) const override { return RegAlias(i); }
  QString regInfo(unsigned i) const override { return RegInfo(i); }
  bool regIsReadOnly(unsigned i) const override { return RegIsReadOnly(i); }
  unsigned int regNumber(const QString &reg, bool &success) const override {
    return RegNumber(reg, success);
  }
};

class RV_ISAInfoBase : public ISAInfoBase {
public:
  RV_ISAInfoBase(const QStringList extensions) {
    // Validate extensions
    for (const auto &ext : extensions) {
      if (supportsExtension(ext)) {
        m_enabledExtensions << ext;
      } else {
        assert(false && "Invalid extension specified for ISA");
      }
    }

    m_regInfos[RegisterFileType::GPR] = std::make_unique<RV_GPRInfo>();

    /** Any registers added by extensions can be applied here
     * Floating point register: */
    // if (extensions.contains("F"))
    //   m_regFileTypes[RegisterFileType::FPR] = std::make_unique<RV_FPRInfo>()
  }

  QString name() const override { return CCmarch().toUpper(); }
  int spReg() const override { return 2; }
  int gpReg() const override { return 3; }
  int syscallReg() const override { return 17; }
  unsigned instrBits() const override { return INSTR_BITS; }
  unsigned elfMachineId() const override { return EM_RISCV; }
  int syscallArgReg(unsigned argIdx) const override {
    assert(argIdx < 8 && "RISC-V only implements argument registers a0-a7");
    return argIdx + 10;
  }

  QString elfSupportsFlags(unsigned flags) const override {
    /** We expect no flags for RV32IM compiled RISC-V executables.
     *  Refer to:
     * https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md#-elf-object-files
     */
    if (flags == 0)
      return QString();
    for (const auto &flag : RVABI::ELFFlagStrings)
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
  QString extensionDescription(const QString &ext) const override {
    if (ext == "M")
      return "Integer multiplication and division";
    if (ext == "C")
      return "Compressed instructions";
    Q_UNREACHABLE();
  }

protected:
  QString _CCmarch(QString march) const {
    // Proceed in canonical order. Canonical ordering is defined in the RISC-V
    // spec.
    for (const auto &ext : {"M", "A", "F", "D", "C"}) {
      if (m_enabledExtensions.contains(ext)) {
        march += QString(ext).toLower();
      }
    }

    return march;
  }

  QStringList m_enabledExtensions;
  QStringList m_supportedExtensions = {"M", "C"};
};

enum OpcodeID {
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
  SYSTEM = 0b1110011,
  AUIPC = 0b0010111,
  INVALID = 0b0
};
enum QuadrantID {
  QUADRANT0 = 0b00,
  QUADRANT1 = 0b01,
  QUADRANT2 = 0b10,
  QUADRANT3 = 0b11
};

/// All RISC-V opcodes are defined as a 7-bit field in bits 0-7 of the
/// instruction
template <OpcodeID opcode>
struct OpPartOpcode
    : public OpPart<static_cast<unsigned>(opcode), BitRange<0, 6>> {};

/// All RISC-V instruction quadrants are defined as a 2-bit field in bits 0-1
/// of the instruction
template <QuadrantID quadrant, unsigned N = 32>
struct OpPartQuadrant
    : public OpPart<static_cast<unsigned>(quadrant), BitRange<0, 1, N>> {};

/// All RISC-V Funct3 opcode parts are defined as a 3-bit field in bits 12-14
/// of the instruction
template <unsigned funct3, unsigned N = 32>
struct OpPartFunct3 : public OpPart<funct3, BitRange<12, 14, N>> {};

/// All RISC-V Funct6 opcode parts are defined as a 6-bit field in bits 26-31
/// of the instruction
template <unsigned funct6, unsigned N = 32>
struct OpPartFunct6 : public OpPart<funct6, BitRange<26, 31, N>> {};

/// All RISC-V Funct7 opcode parts are defined as a 7-bit field in bits 25-31
/// of the instruction
template <unsigned funct7, unsigned N = 32>
struct OpPartFunct7 : public OpPart<funct7, BitRange<25, 31, N>> {};

template <typename RegImpl, unsigned tokenIndex, typename Range>
struct GPR_Reg : public Reg<RegImpl, tokenIndex, Range, RV_GPRInfo> {};

/// The RISC-V Rs1 field contains a source register index.
/// It is defined as a 5-bit field in bits 15-19 of the instruction
template <unsigned tokenIndex>
struct RegRs1
    : public GPR_Reg<RegRs1<tokenIndex>, tokenIndex, BitRange<15, 19>> {
  constexpr static std::string_view Name = "rs1";
};

/// The RISC-V Rs2 field contains a source register index.
/// It is defined as a 5-bit field in bits 20-24 of the instruction
template <unsigned tokenIndex>
struct RegRs2
    : public GPR_Reg<RegRs2<tokenIndex>, tokenIndex, BitRange<20, 24>> {
  constexpr static std::string_view Name = "rs2";
};

/// The RISC-V Rd field contains a destination register index.
/// It is defined as a 5-bit field in bits 7-11 of the instruction
template <unsigned tokenIndex>
struct RegRd : public GPR_Reg<RegRd<tokenIndex>, tokenIndex, BitRange<7, 11>> {
  constexpr static std::string_view Name = "rd";
};

}; // namespace RVISA

} // namespace Ripes
