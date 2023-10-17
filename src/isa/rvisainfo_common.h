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
extern const QStringList RegAliases;
extern const QStringList RegNames;
extern const QStringList RegDescs;
extern const QStringList SupportedExtensions;
extern const QStringList DefaultExtensions;

enum class RVBase : unsigned {
  RV32 = 32,
  RV64 = 64,
  //  RV128
};

enum class Extension { M, A, F, D, C };

template <Extension ext>
constexpr std::string_view extensionName() {
  switch (ext) {
  case Extension::M:
    return "M";
  case Extension::A:
    return "A";
  case Extension::F:
    return "F";
  case Extension::D:
    return "D";
  case Extension::C:
    return "C";
  default:
    return "";
  }
}

template <Extension... Extensions>
static QStringList EnabledExtensions =
    (QStringList() << ... << QString(extensionName<Extensions>()));

template <RVBase Base, Extension... Extensions>
struct RVISAInterface : public ISAInterface<RVISAInterface<Base>> {
  constexpr static ISA ISAID = (Base == RVBase::RV32) ? ISA::RV32I : ISA::RV64I;
  constexpr static unsigned Bits = static_cast<unsigned>(Base);
  constexpr static unsigned InstrBits = 32; // TODO: 16 if RV32E
  constexpr static unsigned InstrByteAlignment =
      RVISAInterface<Base, Extensions...>::extensionEnabled(Extension::C) ? 2
                                                                          : 4;
  constexpr static unsigned ElfMachineID = EM_RISCV;
  constexpr static unsigned RegCnt = 32; // TODO: Extensions can add registers
  constexpr static int GPReg = 2;
  constexpr static int SPReg = 3;
  constexpr static int SyscallReg = 17;

  static const QStringList &SupportedExtensions() {
    return RVISA::SupportedExtensions;
  }
  static const QStringList &EnabledExtensions() {
    return RVISA::EnabledExtensions<Extensions...>;
  }
  static const QStringList &DefaultExtensions() {
    return RVISA::DefaultExtensions;
  }
  constexpr static std::string_view EnabledExtensionNames =
      (extensionName<Extensions>() + ... + "");
  constexpr static bool extensionEnabled(Extension ext) {
    return ((Extensions == ext) || ...);
  }

  static const QStringList &RegNames() { return RVISA::RegNames; }
  static const QStringList &RegAliases() { return RVISA::RegAliases; }
  static const QStringList &RegDescs() { return RVISA::RegDescs; }

  static QString CCmarch() {
    QString march = (Base == RVBase::RV32) ? "rv32i" : "rv64i";
    march += EnabledExtensionNames;
    return march;
  }
  static QString CCmabi() { return (Base == RVBase::RV32) ? "ilp32" : "lp64"; }

  constexpr static bool regIsReadOnly(unsigned i) { return i == 0; }

  static int syscallArgReg(unsigned argIdx) {
    assert(argIdx < 8 && "RISC-V only implements argument registers a0-a7");
    return argIdx + 10;
  }
  static QString elfSupportsFlags(unsigned flags) {
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
  static QString extensionDescription(const QString &ext) {
    if (ext == "M")
      return "Integer multiplication and division";
    if (ext == "C")
      return "Compressed instructions";
    Q_UNREACHABLE();
  }
};

template <RVBase Base>
using RVIM = RVISAInterface<Base, Extension::M>;
template <RVBase Base>
using RVIC = RVISAInterface<Base, Extension::C>;
template <RVBase Base>
using RVIMC = RVISAInterface<Base, Extension::M, Extension::C>;

using RV32I = RVISAInterface<RVBase::RV32>;
using RV32IM = RVIM<RVBase::RV32>;
using RV32IC = RVIC<RVBase::RV32>;
using RV32IMC = RVIMC<RVBase::RV32>;

using RV64I = RVISAInterface<RVBase::RV64>;
using RV64IM = RVIM<RVBase::RV64>;
using RV64IC = RVIC<RVBase::RV64>;
using RV64IMC = RVIMC<RVBase::RV64>;

std::shared_ptr<ISAInfo> constructISA(RVBase base,
                                      const QStringList &extensions);

template <unsigned XLEN>
const std::shared_ptr<ISAInfo> ISAStruct =
    RVISA::RVIMC<static_cast<RVISA::RVBase>(XLEN)>::getStruct();

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
  ECALL = 0b1110011,
  AUIPC = 0b0010111,
  INVALID = 0b0
};
enum QuadrantID {
  QUADRANT0 = 0b00,
  QUADRANT1 = 0b01,
  QUADRANT2 = 0b10,
  QUADRANT3 = 0b11
};

// struct RVISAInterface {
//   static QString regName(unsigned regNumber);
//   static unsigned regNumber(const QString &regToken, bool &success);
// };

/// All RISC-V opcodes are defined as a 7-bit field in bits 0-7 of the
/// instruction
template <unsigned opcode>
struct OpPartOpcode : public OpPart<opcode, BitRange<0, 6>> {};

/// All RISC-V instruction quadrants are defined as a 2-bit field in bits 0-1
/// of the instruction
template <unsigned quadrant>
struct OpPartQuadrant : public OpPart<quadrant, BitRange<0, 1>> {};

/// All RISC-V Funct3 opcode parts are defined as a 3-bit field in bits 12-14
/// of the instruction
template <unsigned funct3>
struct OpPartFunct3 : public OpPart<funct3, BitRange<12, 14>> {};

/// All RISC-V Funct6 opcode parts are defined as a 6-bit field in bits 26-31
/// of the instruction
template <unsigned funct6>
struct OpPartFunct6 : public OpPart<funct6, BitRange<26, 31>> {};

/// All RISC-V Funct7 opcode parts are defined as a 7-bit field in bits 25-31
/// of the instruction
template <unsigned funct7>
struct OpPartFunct7 : public OpPart<funct7, BitRange<25, 31>> {};

/// The RISC-V Rs1 field contains a source register index.
/// It is defined as a 5-bit field in bits 15-19 of the instruction
template <unsigned tokenIndex>
struct RegRs1 : public Reg<tokenIndex, BitRange<15, 19>, RV32I> {
  RegRs1() : Reg<tokenIndex, BitRange<15, 19>, RV32I>("rs1") {}
};

/// The RISC-V Rs2 field contains a source register index.
/// It is defined as a 5-bit field in bits 20-24 of the instruction
template <unsigned tokenIndex>
struct RegRs2 : public Reg<tokenIndex, BitRange<20, 24>, RV32I> {
  RegRs2() : Reg<tokenIndex, BitRange<20, 24>, RV32I>("rs2") {}
};

/// The RISC-V Rd field contains a destination register index.
/// It is defined as a 5-bit field in bits 7-11 of the instruction
template <unsigned tokenIndex>
struct RegRd : public Reg<tokenIndex, BitRange<7, 11>, RV32I> {
  RegRd() : Reg<tokenIndex, BitRange<7, 11>, RV32I>("rd") {}
};

template <typename PseudoInstrImpl>
struct PseudoInstrLoad
    : public PseudoInstruction<PseudoInstrLoad<PseudoInstrImpl>> {
  struct PseudoLoadFields {
    using Reg = PseudoReg<0, RV32I>;
    using Imm = PseudoImm<1>;
    using Impl = FieldsImpl<Reg, Imm>;
  };
  using Fields = PseudoLoadFields;

  constexpr static unsigned ExpectedTokens = 1 + Fields::Impl::numFields();
  static QString mnemonic() { return PseudoInstrImpl::mnemonic(); }
  static Result<std::vector<LineTokens>>
  expander(const PseudoInstruction<PseudoInstrLoad<PseudoInstrImpl>> &,
           const TokenizedSrcLine &line, const SymbolMap &) {
    LineTokensVec v;
    v.push_back(LineTokens() << Token("auipc") << line.tokens.at(1)
                             << Token(line.tokens.at(2), "%pcrel_hi"));
    v.push_back(LineTokens()
                << PseudoInstrImpl::mnemonic() << line.tokens.at(1)
                << Token(QString("(%1 + 4) ").arg(line.tokens.at(2)),
                         "%pcrel_lo")
                << line.tokens.at(1));
    return v;
  }
};
}; // namespace RVISA

} // namespace Ripes
