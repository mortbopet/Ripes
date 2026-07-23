#pragma once

#include "elfio/elf_types.hpp"
#include "instruction.h"
#include "isainfo.h"
#include "rvrelocations.h"

#include <QHash>
#include <functional>
#include <span>
#include <variant>

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

// Width of instructions
constexpr int c_RVInstrWidth = 32;

/* Integer Register File */
// Number of registers
constexpr int c_RVRegs = 32;
// Width of operand to index into registers
constexpr int c_RVRegsBits = vsrtl::ceillog2(c_RVRegs);

/* Floating point Register File */
// Number of registers
constexpr int c_RVFRegs = 32;
// Width of operand to index into registers
constexpr int c_RVFRegsBits = vsrtl::ceillog2(c_RVFRegs);
// Float bit width
constexpr int c_RVFBits = 32;

/* Control and Status Registers */
// Number of CSR registers
constexpr int c_RVCsrRegs = 4096;
// Width of operand to index into CSR registers
constexpr int c_RVCsrRegsBits = vsrtl::ceillog2(c_RVCsrRegs);

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

extern const QStringList FPRRegAliases;
extern const QStringList FPRRegNames;
extern const QStringList FPRRegDescs;

extern const QHash<unsigned, QString> CSRRegAliases;
extern const QHash<unsigned, QString> CSRRegNames;
extern const QHash<unsigned, QString> CSRRegDescs;

template <typename InstrImpl>
struct RV_Instruction : public Instruction<InstrImpl> {
  constexpr static unsigned instrBits() { return c_RVInstrWidth; }
};

constexpr std::string_view GPR = "gpr";
constexpr std::string_view FPR = "fpr";
constexpr std::string_view CSR = "csr";

constexpr std::string_view GPR_DESC = "General purpose registers";
constexpr std::string_view FPR_DESC = "Floating-point registers";
constexpr std::string_view CSR_DESC = "Control and status registers";

struct RV_RegFileInterface : public RegFileInfoInterface {
  RV_RegFileInterface(const ISAInfoBase *isaInfo) : m_isaInfo(isaInfo){};

  unsigned bits() const override { return m_isaInfo ? m_isaInfo->bits() : 32; }

protected:
  const ISAInfoBase *m_isaInfo;
};

/// Defines information about the general RISC-V register file.
struct RV_GPRInfo : public RV_RegFileInterface {
  RV_GPRInfo() : RV_RegFileInterface(nullptr){};
  RV_GPRInfo(const ISAInfoBase *isaInfo) : RV_RegFileInterface(isaInfo){};

  std::string_view regFileName() const override { return GPR; }
  std::string_view regFileDesc() const override { return GPR_DESC; }
  unsigned int regCnt() const override { return c_RVRegs; }
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
struct RV_FPRInfo : public RV_RegFileInterface {
  RV_FPRInfo() : RV_RegFileInterface(nullptr){};
  RV_FPRInfo(const ISAInfoBase *isaInfo) : RV_RegFileInterface(isaInfo){};

  unsigned bits() const override { return 32; }

  std::string_view regFileName() const override { return FPR; }
  std::string_view regFileDesc() const override { return FPR_DESC; }
  unsigned int regCnt() const override { return c_RVFRegs; }
  QString regName(unsigned i) const override {
    return RVISA::FPRRegNames.size() > static_cast<int>(i)
               ? RVISA::FPRRegNames.at(static_cast<int>(i))
               : QString();
  }
  QString regAlias(unsigned i) const override {
    return RVISA::FPRRegAliases.size() > static_cast<int>(i)
               ? RVISA::FPRRegAliases.at(static_cast<int>(i))
               : QString();
  }
  QString regInfo(unsigned i) const override {
    return RVISA::FPRRegDescs.size() > static_cast<int>(i)
               ? RVISA::FPRRegDescs.at(static_cast<int>(i))
               : QString();
  }
  bool regIsReadOnly(unsigned) const override { return false; }
  unsigned int regNumber(const QString &reg, bool &success) const override {
    success = true;

    if (RVISA::FPRRegNames.contains(reg)) {
      return RVISA::FPRRegNames.indexOf(reg);
    }

    if (RVISA::FPRRegAliases.contains(reg)) {
      return RVISA::FPRRegAliases.indexOf(reg);
    }

    success = false;

    return 0;
  }
};

/// Defines information about the floating-point RISC-V register file.
struct RV_CSRInfo : public RV_RegFileInterface {
  // clang-format off
  enum class CSR : unsigned {
    FFLAGS = 0x001,
    FRM    = 0x002,
    FCSR   = 0x003
  };
  // count of the actual implemented CSRs and not the entire addressable space
  // Attention: Must always be in sync with the enum class CSR above
  constexpr static unsigned CSRCount = 3;
  // clang-format on

  RV_CSRInfo() : RV_RegFileInterface(nullptr){};
  RV_CSRInfo(const ISAInfoBase *isaInfo) : RV_RegFileInterface(isaInfo){};

  std::string_view regFileName() const override { return RVISA::CSR; }
  std::string_view regFileDesc() const override { return CSR_DESC; }
  unsigned int regCnt() const override { return CSRCount; }
  QString regName(unsigned i) const override {
    return RVISA::CSRRegNames.value(i, QString());
  }
  QString regAlias(unsigned i) const override {
    return RVISA::CSRRegAliases.value(i, QString());
  }
  QString regInfo(unsigned i) const override {
    return RVISA::CSRRegDescs.value(i, QString());
  }
  bool regIsReadOnly(unsigned i) const override {
    // typically csrs use the highest 4 bits to encode the accessibility
    // where 0b11 in the 2 most significant bits indicates read-only access
    // since for now we dont have any special csrs defined this simple check
    // suffices
    return ((i >> 10) & 0b11) == 0b11;
  }
  unsigned int regNumber(const QString &reg, bool &success) const override {
    int key;
    success = true;

    key = static_cast<int>(RVISA::CSRRegNames.key(reg, -1));
    if (key != static_cast<int>(-1)) {
      return key;
    }

    key = static_cast<int>(RVISA::CSRRegAliases.key(reg, -1));
    if (key != static_cast<int>(-1)) {
      return key;
    }

    success = false;

    return 0;
  }
};

//------------------------------------------------------------------------------
// EXTENSION MANAGEMENT
//------------------------------------------------------------------------------

enum class Option {
  shifts64BitVariant, // appends 'w' to 32-bit shift operations, for use in
                      // the 64-bit RISC-V ISA
  LI64BitVariant      // Modifies LI to be able to emit 64-bit constants
};

namespace ExtI {
void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions,
               const std::set<Option> &options = {});
}

namespace ExtM {
void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);
}

namespace ExtC {
void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);
}

namespace ExtF {
void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);
}

namespace ExtZicsr {
void enableExt(const ISAInfoBase *isa, InstrVec &instructions,
               PseudoInstrVec &pseudoInstructions);
}

struct RV_Extension;
class RV_ExtensionSet : public ExtensionSetInfo {
public:
  // since the the ExtensionSet does not own the extensions it contains, we must
  // not free them in the destructor
  ~RV_ExtensionSet(){};

  RV_ExtensionSet() = default;
  RV_ExtensionSet(const RV_ExtensionSet &other) = default;
  RV_ExtensionSet(RV_ExtensionSet &&other) = default;
  RV_ExtensionSet &operator=(const RV_ExtensionSet &other) = default;
  RV_ExtensionSet &operator=(RV_ExtensionSet &&other) = default;

  template <typename... Exts,
            typename = std::enable_if_t<
                (std::is_base_of_v<ExtensionInfoInterface, Exts> && ...)>>
  RV_ExtensionSet(const Exts &...extensions)
      : m_extensions{
            static_cast<const ExtensionInfoInterface *>(&extensions)...} {}

  /// Conversion constructor from the polymorphic base class (e.g. when
  /// ExtensionSetInfo& is passed through a generic interface but actually
  /// refers to an RV_ExtensionSet).
  explicit RV_ExtensionSet(const ExtensionSetInfo &other) {
    for (const auto *ext : other.extensions()) {
      m_extensions.insert(ext);
    }
  }

  const ExtensionContainer_t &extensions() const override {
    return m_extensions;
  }

  QString CCmarch() const override;

  ExtensionSetInfo::UniquePtr clone() const override {
    return std::make_unique<RV_ExtensionSet>(*this);
  }
  RV_ExtensionSet &operator<<(const ExtensionInfoInterface &ext) override;
  const ExtensionInfoInterface *
  remove(const ExtensionInfoInterface &ext) override;

private:
  ExtensionContainer_t m_extensions;
};

using FuncEnableExt =
    std::function<void(const ISAInfoBase *, InstrVec &, PseudoInstrVec &)>;
using FuncEnableExtOption =
    std::function<void(const ISAInfoBase *, InstrVec &, PseudoInstrVec &,
                       const std::set<Option> &)>;
struct RV_Extension : public ExtensionInfoInterface {
  ExtensionID_t m_id;
  QString m_name;
  QString m_description;
  RV_ExtensionSet m_implicates;

  std::variant<FuncEnableExt, FuncEnableExtOption> enableFunc;

  ExtensionID_t id() const override { return m_id; }
  QString name() const override { return m_name; }
  QString description() const override { return m_description; }

  QString CCmarchName() const override { return m_name; }

  const RV_ExtensionSet &implicates() const override { return m_implicates; }

  void enableInstructions(const ISAInfoBase *isa, InstrVec &instructions,
                          PseudoInstrVec &pseudoInstructions,
                          const std::set<Option> &options = {}) const {
    if (std::holds_alternative<FuncEnableExtOption>(enableFunc)) {
      std::get<FuncEnableExtOption>(enableFunc)(isa, instructions,
                                                pseudoInstructions, options);
    } else {
      std::get<FuncEnableExt>(enableFunc)(isa, instructions,
                                          pseudoInstructions);
    }
  }

private:
  // only the ExtensionManager class can create extensions,
  // this ensures that all extensions are predefined and prevents accidental
  // creation of invalid extensions outside this controlled environment
  friend class ExtensionManager;
  template <typename ID_t = ExtensionID_t>
  RV_Extension(
      ID_t id, QString name, QString description, RV_ExtensionSet implicates,
      std::variant<FuncEnableExt, FuncEnableExtOption> enableFunc) noexcept
      : m_id(static_cast<ExtensionID_t>(id)), m_name(std::move(name)),
        m_description(std::move(description)),
        m_implicates(std::move(implicates)), enableFunc(std::move(enableFunc)) {
  }
};

/// Instantiates and holds all predefined RISC-V extensions, as defined in the
/// ISA specification, in canonical order.
class ExtensionManager {
public:
  enum class Id : ExtensionID_t {
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // ! must be in canonical order !
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    I = 0, // Base Integer ISA
    M,     // Integer Multiplication and Division
    F,     // Single-Precision Floating-Point
    C,     // Compressed Instructions
    Zicsr, // Control and Status Register Instructions

    EXTENSION_COUNT
  };

  // clang-format off
  static inline const RV_Extension I     = RV_Extension( Id::I, "I", "Base Integer ISA", {}, ExtI::enableExt );
  static inline const RV_Extension M     = RV_Extension( Id::M, "M", "Integer multiplication and division", {}, ExtM::enableExt );
  static inline const RV_Extension C     = RV_Extension( Id::C, "C", "Compressed Instructions", {}, ExtC::enableExt );
  static inline const RV_Extension Zicsr = RV_Extension( Id::Zicsr, "Zicsr", "Control and Status Register Instructions", {}, ExtZicsr::enableExt );
  static inline const RV_Extension F     = RV_Extension( Id::F, "F", "Single-Precision Floating-Point Instructions ", {Zicsr}, ExtF::enableExt );
  // clang-format on

  static inline const RV_ExtensionSet all{M, F, C, Zicsr};
};
using Extension = ExtensionManager;

//------------------------------------------------------------------------------
// RISC-V ISA MANAGEMENT
//------------------------------------------------------------------------------

class RV_ISAInfoBase : public ISAInfoBase {
public:
  using ISAInfoBase::CCmarch; // circumvent name shadowing of overloaded
                              // function

  static const RV_ExtensionSet &getSupportedExtensions() {
    return Extension::all;
  }
  static const RV_ExtensionSet &getDefaultExtensions() {
    static const RV_ExtensionSet exts{Extension::M};
    return exts;
  }

  RV_ISAInfoBase() : RV_ISAInfoBase(RV_ExtensionSet()) {}
  RV_ISAInfoBase(const ExtensionSetInfo &extensions)
      : RV_ISAInfoBase(RV_ExtensionSet(extensions)) {}
  RV_ISAInfoBase(RV_ExtensionSet extensions) {
    // Validate extensions
    if (extensions.isSubsetOf(getSupportedExtensions())) {
      m_enabledExtensions = std::move(extensions);
    } else {
      assert(false && "Invalid extension specified for ISA");
    }

    m_regInfos[GPR] = std::make_unique<RV_GPRInfo>(this);
    if (extensionEnabled(Extension::F)) {
      m_regInfos[FPR] = std::make_unique<RV_FPRInfo>(this);
    }

    // as for now the f extension manages the fcsr it self
    // and therefor we dont use the CSR regfile
    // if (extensionEnabled(Extension::Zicsr)) {
    //   m_regInfos[CSR] = std::make_unique<RV_CSRInfo>(this);
    // }

    // Setup relocations
    m_relocations = rvRelocations();
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
  unsigned instrBits() const override { return c_RVInstrWidth; }
  unsigned elfMachineId() const override { return EM_RISCV; }
  std::optional<RegIndex> syscallArgReg(unsigned argIdx) const override {
    assert(argIdx < 8 && "RISC-V only implements argument registers a0-a7");
    return RegIndex{m_regInfos.at(GPR), argIdx + 10};
  }

  QString CCmarch(const ExtensionSetInfo &extensions) const override {
    return _CCmarchPrefix() + extensions.CCmarch();
  }

  QString elfSupportsFlags(unsigned flags) const override {
    /** We expect no flags for RV32IM compiled RISC-V executables.
     *  Refer to:
     * https://github.com/riscv-non-isa/riscv-elf-psabi-doc/blob/master/riscv-elf.adoc#elf-object-files
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

  const RV_ExtensionSet &supportedExtensions() const override {
    return m_supportedExtensions;
  }
  const RV_ExtensionSet &enabledExtensions() const override {
    return m_enabledExtensions;
  }

  const InstrVec &instructions() const override { return m_instructions; }
  const PseudoInstrVec &pseudoInstructions() const override {
    return m_pseudoInstructions;
  }
  const RelocationsVec &relocations() const override { return m_relocations; }

protected:
  QString _CCmarchPrefix() const {
    // i or e base isa extension is determined by the presence of the I or E
    // extension in enabledExtensions
    return "rv" + QString::number(bits()) +
           (isaID() == ISA::RV32I || isaID() == ISA::RV64I ? "I" : "E");
  }

  void _loadExtensionSet(const RV_ExtensionSet &extSet,
                         const std::set<Option> &options = {}) {
    for (const auto *ext : extSet.extensions()) {
      if (const RV_Extension *rvExt = dynamic_cast<const RV_Extension *>(ext)) {
        m_loadedExtensions << rvExt->id();

        rvExt->enableInstructions(this, m_instructions, m_pseudoInstructions,
                                  options);

        _loadExtensionSet(rvExt->implicates(), options);
      } else {
        Q_ASSERT_X(false, "RV_ISAInfoBase::_loadExtensionSet",
                   "Only RV_Extensions can be added to an RV_ExtensionSet");
      }
    }
  }

  /// Make sure to call this in any child class's constructor
  void initialize(const RV_Extension &baseExt = Extension::I,
                  const std::set<Option> &options = {}) {
    baseExt.enableInstructions(this, m_instructions, m_pseudoInstructions,
                               options);
    _loadExtensionSet(enabledExtensions(), options);
  }

  QList<ExtensionID_t> m_loadedExtensions;
  RV_ExtensionSet m_enabledExtensions;
  RV_ExtensionSet m_supportedExtensions = getSupportedExtensions();

  RegInfoMap m_regInfos;
  InstrVec m_instructions;
  PseudoInstrVec m_pseudoInstructions;
  RelocationsVec m_relocations;
};

// clang-format off
enum OpcodeID {
  // Base I
  LUI     = 0b0110111,
  JAL     = 0b1101111,
  JALR    = 0b1100111,
  BRANCH  = 0b1100011,
  LOAD    = 0b0000011,
  STORE   = 0b0100011,
  OPIMM   = 0b0010011,
  OP      = 0b0110011,
  OPIMM32 = 0b0011011,
  OP32    = 0b0111011,
  SYSTEM  = 0b1110011,
  AUIPC   = 0b0010111,
  INVALID = 0b0,

  // Fused Multiply and Add
  MADD  = 0b1000011,
  MSUB  = 0b1000111,
  NMSUB = 0b1001011,
  NMADD = 0b1001111,

  // Floating point extension
  LOAD_FP  = 0b0000111,
  STORE_FP = 0b0100111,
  OP_FP    = 0b1010011
};
// clang-format on
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

template <typename RegImpl, unsigned tokenIndex, typename Range>
struct FPR_Reg : public Reg<RegImpl, tokenIndex, Range, RV_FPRInfo> {};

template <typename RegImpl, unsigned tokenIndex, typename Range>
struct CSR_Reg : public Reg<RegImpl, tokenIndex, Range, RV_CSRInfo> {};

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
