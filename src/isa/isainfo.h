#pragma once

#include "limits.h"
#include <QMap>
#include <QSet>
#include <QString>
#include <memory>
#include <set>

#include "elfio/elf_types.hpp"

namespace Ripes {

/// List of currently supported ISAs.
enum class ISA { RV32I, RV64I, MIPS32I };
const static std::map<ISA, QString> ISAFamilyNames = {
    {ISA::RV32I, "RISC-V"}, {ISA::RV64I, "RISC-V"}, {ISA::MIPS32I, "MIPS"}};
enum class RegisterFileType { GPR, FPR, CSR, NoRegisters };
struct RegisterFileName {
  QString shortName;
  QString longName;
};

/// Maintain a mapping between register file enum's and their string
/// representation.
const static std::map<RegisterFileType, RegisterFileName> s_RegsterFileName = {
    {RegisterFileType::GPR, {"GPR", "General purpose registers"}},
    {RegisterFileType::FPR, {"FPR", "Floating-point registers"}},
    {RegisterFileType::CSR, {"CSR", "Control and status registers"}}};

struct ISAInfo {
  const QString name;
  const QString CCmarch;
  const QString CCmabi;
  const QStringList supportedExtensions;
  const QStringList enabledExtensions;
  const QStringList defaultExtensions;
  ISA isaID;
  unsigned regCnt;
  unsigned bits;
  unsigned instrBits;
  unsigned instrByteAlignment;
  int spReg;
  int gpReg;
  int syscallReg;
  unsigned elfMachineId;
  std::function<QString(unsigned)> regName;
  std::function<unsigned(const QString &, bool &)> regNumber;
  std::function<QString(unsigned)> regAlias;
  std::function<QString(unsigned)> regInfo;
  std::function<int(unsigned)> syscallArgReg;
  std::function<bool(unsigned)> regIsReadOnly;
  std::function<bool(const QString &)> extensionEnabled;
  std::function<QString(unsigned)> elfSupportsFlags;
  std::function<QString(const QString &)> extensionDescription;

  unsigned bytes() const { return bits / 8; }
  unsigned instrBytes() const { return instrBits / 8; }
  bool eq(const ISAInfo &otherISA) const {
    const auto ext1 = QSet(enabledExtensions.begin(), enabledExtensions.end());
    const auto ext2 = QSet(otherISA.enabledExtensions.begin(),
                           otherISA.enabledExtensions.end());
    return name == otherISA.name && ext1 == ext2;
  }
};

template <typename ISAImpl>
struct ISAInterface {
  static std::shared_ptr<ISAInfo> getStruct() {
    return std::make_shared<ISAInfo>(ISAInfo{name(),
                                             ISAImpl::CCmarch(),
                                             ISAImpl::CCmabi(),
                                             supportedExtensions(),
                                             enabledExtensions(),
                                             defaultExtensions(),
                                             isaID(),
                                             regCnt(),
                                             bits(),
                                             instrBits(),
                                             instrByteAlignment(),
                                             spReg(),
                                             gpReg(),
                                             syscallReg(),
                                             elfMachineId(),
                                             regName,
                                             regNumber,
                                             regAlias,
                                             regInfo,
                                             ISAImpl::syscallArgReg,
                                             ISAImpl::regIsReadOnly,
                                             extensionEnabled,
                                             ISAImpl::elfSupportsFlags,
                                             ISAImpl::extensionDescription});
  }

  static QString name() { return ISAImpl::CCmarch().toUpper(); }
  constexpr static ISA isaID() { return ISAImpl::ISAID; }

  /// Returns the number of registers in the instruction set.
  constexpr static unsigned regCnt() { return ISAImpl::RegCnt; }
  /// Returns the canonical name of the i'th register in the ISA.
  static QString regName(unsigned i) {
    return ISAImpl::RegNames().size() > static_cast<int>(i)
               ? ISAImpl::RegNames().at(static_cast<int>(i))
               : QString();
  }
  /// Returns the register index for a register name. If regName is not part of
  /// the ISA, sets success to false.
  static unsigned int regNumber(const QString &reg, bool &success) {
    QString regRes = reg;
    success = true;
    if (reg[0] == 'x' && (ISAImpl::RegNames().count(reg) != 0)) {
      regRes.remove('x');
      return regRes.toInt(&success, 10);
    } else if (ISAImpl::RegAliases().contains(reg)) {
      return ISAImpl::RegAliases().indexOf(reg);
    }
    success = false;
    return 0;
  }
  /// Returns the alias name of the i'th register in the ISA. If no alias is
  /// present, should return regName(i).
  static QString regAlias(unsigned i) {
    return ISAImpl::RegAliases().size() > static_cast<int>(i)
               ? ISAImpl::RegAliases().at(static_cast<int>(i))
               : QString();
  }
  /// Returns additional information about the i'th register, i.e. caller/calle
  /// saved info, stack register ...
  static QString regInfo(unsigned i) {
    return ISAImpl::RegDescs().size() > static_cast<int>(i)
               ? ISAImpl::RegDescs().at(static_cast<int>(i))
               : QString();
  }
  /// Returns if the i'th register is read-only.
  //  static bool regIsReadOnly(unsigned i);
  /// Register width, in bits
  constexpr static unsigned bits() { return ISAImpl::Bits; }
  /// Register width, in bytes
  constexpr unsigned bytes() const { return bits() / CHAR_BIT; }
  /// Instruction width, in bits
  constexpr static unsigned instrBits() { return ISAImpl::InstrBits; }
  /// Instruction width, in bytes
  constexpr static unsigned instrBytes() { return instrBits() / CHAR_BIT; }
  /// Instruction Alignment, in bytes
  constexpr static unsigned instrByteAlignment() {
    return ISAImpl::InstrByteAlignment;
  }
  /// Stack pointer
  constexpr static int spReg() { return ISAImpl::SPReg; }
  /// Global pointer
  constexpr static int gpReg() { return ISAImpl::GPReg; }
  /// Syscall function register
  constexpr static int syscallReg() { return ISAImpl::SyscallReg; }
  /// Mapping between syscall argument # and the corresponding register #
  /// wherein that argument is passed.
  //  static int syscallArgReg(unsigned argIdx);

  // GCC Compile command architecture and ABI specification strings
  //  static QString CCmarch();
  //  static QString CCmabi();
  constexpr static unsigned elfMachineId() { return ISAImpl::ElfMachineID; }

  /**
   * @brief elfSupportsFlags
   * The instructcion set should determine whether the provided @p flags, as
   * retrieved from an ELF file, are valid flags for the instruction set. If a
   * mismatch is found, an error message describing the mismatch is returned.
   * Else, returns an empty QString(), validating the flags.
   */
  //  static QString elfSupportsFlags(unsigned flags);

  /**
   * @brief supportedExtensions/enabledExtensions
   * An ISA may have a set of (optional) extensions which may be
   * enabled/disabled for a given processor. SupportedExtensions can be used
   * when ie. instantiating a processor and enabledExtensions when instantiating
   * an assembler for a given processor.
   */
  static const QStringList &enabledExtensions() {
    return ISAImpl::EnabledExtensions();
  }
  static bool extensionEnabled(const QString &ext) {
    return ISAImpl::enabledExtensions().contains(ext);
  }
  static const QStringList &supportedExtensions() {
    return ISAImpl::SupportedExtensions();
  }
  static bool supportsExtension(const QString &ext) {
    return ISAImpl::supportedExtensions().contains(ext);
  }
  static const QStringList &defaultExtensions() {
    return ISAImpl::DefaultExtensions();
  }
  //  static QString extensionDescription(const QString &ext);

  /**
   * ISA equality is defined as a separate function rather than the == operator,
   * given that we might need to check for ISA equivalence, without having
   * instantiated the other ISA. As such, it being uninstantiated does not allow
   * comparison of extensions.
   */
  template <typename OtherISAInterface>
  bool eq() const {
    const auto ext1 = QSet(ISAImpl::enabledExtensions().begin(),
                           ISAImpl::enabledExtensions().end());
    const auto ext2 = QSet(OtherISAInterface::enabledExtensions().begin(),
                           OtherISAInterface::enabledExtensions().end());
    return ISAImpl::name() == OtherISAInterface::name() && ext1 == ext2;
  }
};

} // namespace Ripes
