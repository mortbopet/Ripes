#pragma once

#include "elfio/elf_types.hpp"
#include "instruction.h"
#include "isainfo.h"
#include "rvrelocations.h"

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

template <ISA isa>
class ISAVliwInfo; // forward declaration for RV_ISAInfoBase

namespace RVISA {
extern const QStringList GPRRegAliases;
extern const QStringList GPRRegNames;
extern const QStringList GPRRegDescs;

constexpr unsigned INSTR_BITS = 32;

template <typename InstrImpl>
struct RV_Instruction : public Instruction<InstrImpl> {
  constexpr static unsigned instrBits() { return INSTR_BITS; }
};

constexpr std::string_view GPR = "gpr";
constexpr std::string_view FPR = "fpr";
constexpr std::string_view CSR = "csr";

constexpr std::string_view GPR_DESC = "General purpose registers";
constexpr std::string_view FPR_DESC = "Floating-point registers";
constexpr std::string_view CSR_DESC = "Control and status registers";

/// Defines information about the general RISC-V register file.
struct RV_GPRInfo : public RegFileInfoInterface {
  std::string_view regFileName() const override { return GPR; }
  std::string_view regFileDesc() const override { return GPR_DESC; }
  unsigned int regCnt() const override { return 32; }
  QString regName(unsigned i) const override {
    return RVISA::GPRRegNames.size() > static_cast<int>(i)
               ? RVISA::GPRRegNames.at(static_cast<int>(i))
               : QString();
  }
  QString regAlias(unsigned i) const override {
    return RVISA::GPRRegAliases.size() > static_cast<int>(i)
               ? RVISA::GPRRegAliases.at(static_cast<int>(i))
               : QString();
  }
  QString regInfo(unsigned i) const override {
    return RVISA::GPRRegDescs.size() > static_cast<int>(i)
               ? RVISA::GPRRegDescs.at(static_cast<int>(i))
               : QString();
  }
  bool regIsReadOnly(unsigned i) const override { return i == 0; }
  unsigned int regNumber(const QString &reg, bool &success) const override {
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
};

/// Defines information about the floating-point RISC-V register file.
struct RV_FPRInfo : public RegFileInfoInterface {
  std::string_view regFileName() const override { return FPR; }
  std::string_view regFileDesc() const override { return FPR_DESC; }
  // TODO: Fill out RISC-V floating point register info
  unsigned int regCnt() const override { return 0; }
  QString regName(unsigned) const override { return QString(); }
  QString regAlias(unsigned) const override { return QString(); }
  QString regInfo(unsigned) const override { return QString(); }
  bool regIsReadOnly(unsigned) const override { return false; }
  unsigned int regNumber(const QString &, bool &success) const override {
    success = false;
    return 0;
  }
};

enum class ImplementationDetail {
  shifts64BitVariant,      // appends 'w' to 32-bit shift operations, for use in
                           // the 64-bit RISC-V ISA
  LI64BitVariant,          // Modifies LI to be able to emit 64-bit constants
  VliwPseudoInstrExpansion // notifies the isa to use a special pseudo
                           // instruction for the vliw processor
};
using ImplDetails = std::set<ImplementationDetail>;

namespace ExtI {
void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions,
               const ImplDetails &details = {});
}

namespace ExtM {
void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);
}

namespace ExtC {
void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);
}

class RV_ISAInfoBase : public ISAInfoBase {
public:
  static const QStringList &getSupportedExtensions() {
    static const QStringList ext = {"M", "C"};
    return ext;
  }
  static const QStringList &getDefaultExtensions() {
    static const QStringList ext = {"M"};
    return ext;
  }

  RV_ISAInfoBase(const QStringList extensions) {
    // Validate extensions
    for (const auto &ext : extensions) {
      if (supportsExtension(ext)) {
        m_enabledExtensions << ext;
      } else {
        assert(false && "Invalid extension specified for ISA");
      }
    }

    m_regInfos[GPR] = std::make_unique<RV_GPRInfo>();
    if (supportsExtension("F")) {
      m_regInfos[FPR] = std::make_unique<RV_FPRInfo>();
    }
  }

  const RegInfoMap &regInfoMap() const override { return m_regInfos; }

  QString name() const override { return CCmarch().toUpper(); }
  std::optional<RegIndex> spReg() const override {
    return RegIndex{m_regInfos.at(GPR), 2};
  }
  std::optional<RegIndex> gpReg() const override {
    return RegIndex{m_regInfos.at(GPR), 3};
  }
  std::optional<RegIndex> syscallReg() const override {
    return RegIndex{m_regInfos.at(GPR), 17};
  }
  unsigned instrBits() const override { return INSTR_BITS; }
  unsigned elfMachineId() const override { return EM_RISCV; }
  std::optional<RegIndex> syscallArgReg(unsigned argIdx) const override {
    assert(argIdx < 8 && "RISC-V only implements argument registers a0-a7");
    return RegIndex{m_regInfos.at(GPR), argIdx + 10};
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

  const InstrVec &instructions() const override { return m_instructions; }
  const PseudoInstrVec &pseudoInstructions() const override {
    return m_pseudoInstructions;
  }
  const RelocationsVec &relocations() const override { return m_relocations; }

protected:
  /**
   * @brief interface for customizing initialization of instructions
   * usefull for implementing custom Implementation Details via
   * m_enabledDetails, which e.g. have special relocations or pseudo instruction
   * expansions
   */
  void loadInstructionSet() {
    // Setup relocations
    m_relocations = rvRelocations();

    RVISA::ExtI::enableExt(this, m_instructions, m_pseudoInstructions,
                           m_enabledDetails);
    for (const auto &extension : m_enabledExtensions) {
      switch (extension.unicode()->toLatin1()) {
      case 'M':
        RVISA::ExtM::enableExt(this, m_instructions, m_pseudoInstructions);
        break;
      case 'C':
        RVISA::ExtC::enableExt(this, m_instructions, m_pseudoInstructions);
        break;
      }
    }
  }
  /**
   * @brief reset loaded Instructions, Pseudo instructions, and relocations.
   * @note used as inverse to loadInstructionSet()
   */
  void unLoadInstructionSet() {
    m_instructions.clear();
    m_pseudoInstructions.clear();
    m_relocations.clear();
  }

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
  QStringList m_supportedExtensions = getSupportedExtensions();

  ImplDetails m_enabledDetails = {};

  RegInfoMap m_regInfos;
  InstrVec m_instructions;
  PseudoInstrVec m_pseudoInstructions;
  RelocationsVec m_relocations;

  template <ISA isa>
  friend class ::Ripes::ISAVliwInfo;
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
  constexpr static std::string_view getName() { return "rs1"; }
};

/// The RISC-V Rs2 field contains a source register index.
/// It is defined as a 5-bit field in bits 20-24 of the instruction
template <unsigned tokenIndex>
struct RegRs2
    : public GPR_Reg<RegRs2<tokenIndex>, tokenIndex, BitRange<20, 24>> {
  constexpr static std::string_view getName() { return "rs2"; }
};

/// The RISC-V Rd field contains a destination register index.
/// It is defined as a 5-bit field in bits 7-11 of the instruction
template <unsigned tokenIndex>
struct RegRd : public GPR_Reg<RegRd<tokenIndex>, tokenIndex, BitRange<7, 11>> {
  constexpr static std::string_view getName() { return "rd"; }
};

}; // namespace RVISA

} // namespace Ripes
