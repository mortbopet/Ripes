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
enum class RegisterFileType { GPR, FPR, CSR };
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

/// Description of an interface into one or more register information structs
struct RegInfoInterface {
  /// Returns the number of registers in the instruction set.
  virtual unsigned regCnt() const = 0;
  /// Returns the canonical name of the i'th register in the ISA.
  virtual QString regName(unsigned i) const = 0;
  /// Returns the register index for a register name. If regName is not part of
  /// the ISA, sets success to false.
  virtual unsigned regNumber(const QString &regName, bool &success) const = 0;
  /// Returns the alias name of the i'th register in the ISA. If no alias is
  /// present, should return regName(i).
  virtual QString regAlias(unsigned i) const = 0;
  /// Returns additional information about the i'th register, i.e. caller/calle
  /// saved info, stack register ...
  virtual QString regInfo(unsigned i) const = 0;
  /// Returns if the i'th register is read-only.
  virtual bool regIsReadOnly(unsigned i) const = 0;
};

/// Description of an interface into a single register information struct
struct RegInfoBase : public RegInfoInterface {
  virtual RegisterFileType regFileType() const = 0;
};

using RegisterInfoMap =
    std::map<RegisterFileType, std::shared_ptr<const RegInfoBase>>;

/// A set of all the registers' information in an ISA
struct RegInfoSet : public RegInfoInterface {
  RegisterInfoMap map;

  std::shared_ptr<const RegInfoBase> &operator[](const RegisterFileType key) {
    return map[key];
  }

  unsigned regCnt() const {
    unsigned count = 0;
    for (const auto &reg : map) {
      count += reg.second->regCnt();
    }
    return count;
  }
  QString regName(unsigned i) const {
    if (i < regCnt())
      return getIndexedInfo(i)->regName(i);
    else
      return QString();
  }
  unsigned regNumber(const QString &regName, bool &success) const {
    unsigned count = 0;
    for (const auto &reg : map) {
      bool innerSuccess = false;
      if (unsigned regNum = reg.second->regNumber(regName, innerSuccess);
          innerSuccess) {
        success = true;
        return regNum + count;
      }
      count += reg.second->regCnt();
    }
    success = false;
    return 0;
  }
  QString regAlias(unsigned i) const {
    if (i < regCnt())
      return getIndexedInfo(i)->regAlias(i);
    else
      return QString();
  }
  QString regInfo(unsigned i) const {
    if (i < regCnt())
      return getIndexedInfo(i)->regInfo(i);
    else
      return QString();
  }
  bool regIsReadOnly(unsigned i) const {
    if (i < regCnt())
      return getIndexedInfo(i)->regIsReadOnly(i);
    else
      return false;
  }

private:
  const RegInfoBase *getIndexedInfo(unsigned i) const {
    assert(i < regCnt() && "Register index out of range");
    unsigned count = 0;
    for (const auto &reg : map) {
      if (i < count + reg.second->regCnt()) {
        return reg.second.get();
      }
      count += reg.second->regCnt();
    }

    Q_UNREACHABLE();
  }
};

/// The ISAInfoBase class defines an interface for instruction set information.
class ISAInfoBase : public RegInfoInterface {
public:
  virtual ~ISAInfoBase(){};
  virtual QString name() const = 0;
  virtual ISA isaID() const = 0;

  QString regName(unsigned i) const override { return m_regInfoSet.regName(i); }
  unsigned regNumber(const QString &regName, bool &success) const override {
    return m_regInfoSet.regNumber(regName, success);
  }
  QString regAlias(unsigned i) const override {
    return m_regInfoSet.regAlias(i);
  }
  QString regInfo(unsigned i) const override { return m_regInfoSet.regInfo(i); }
  bool regIsReadOnly(unsigned i) const override {
    return m_regInfoSet.regIsReadOnly(i);
  }
  unsigned regCnt() const override { return m_regInfoSet.regCnt(); }

  std::optional<const RegInfoBase *>
  regInfo(RegisterFileType regFileType) const {
    if (auto match = m_regInfoSet.map.find(regFileType);
        match != m_regInfoSet.map.end()) {
      return m_regInfoSet.map.at(regFileType).get();
    } else {
      return {};
    }
  }

  virtual unsigned bits() const = 0; // Register width, in bits
  unsigned bytes() const {
    return bits() / CHAR_BIT;
  }                                       // Register width, in bytes
  virtual unsigned instrBits() const = 0; // Instruction width, in bits
  unsigned instrBytes() const {
    return instrBits() / CHAR_BIT;
  } // Instruction width, in bytes
  virtual unsigned instrByteAlignment() const {
    return 0;
  }                                        // Instruction Alignment, in bytes
  virtual int spReg() const { return -1; } // Stack pointer
  virtual int gpReg() const { return -1; } // Global pointer
  virtual int syscallReg() const { return -1; } // Syscall function register
  // Mapping between syscall argument # and the corresponding register # wherein
  // that argument is passed.
  virtual int syscallArgReg(unsigned /*argIdx*/) const { return -1; }

  // GCC Compile command architecture and ABI specification strings
  virtual QString CCmarch() const = 0;
  virtual QString CCmabi() const = 0;
  virtual unsigned elfMachineId() const = 0;

  /**
   * @brief elfSupportsFlags
   * The instructcion set should determine whether the provided @p flags, as
   * retrieved from an ELF file, are valid flags for the instruction set. If a
   * mismatch is found, an error message describing the mismatch is returned.
   * Else, returns an empty QString(), validating the flags.
   */
  virtual QString elfSupportsFlags(unsigned flags) const = 0;

  /**
   * @brief supportedExtensions/enabledExtensions
   * An ISA may have a set of (optional) extensions which may be
   * enabled/disabled for a given processor. SupportedExtensions can be used
   * when ie. instantiating a processor and enabledExtensions when instantiating
   * an assembler for a given processor.
   */
  virtual const QStringList &supportedExtensions() const = 0;
  virtual const QStringList &enabledExtensions() const = 0;
  bool extensionEnabled(const QString &ext) const {
    return enabledExtensions().contains(ext);
  }
  bool supportsExtension(const QString &ext) const {
    return supportedExtensions().contains(ext);
  }
  virtual QString extensionDescription(const QString &ext) const = 0;

  /**
   * ISA equality is defined as a separate function rather than the == operator,
   * given that we might need to check for ISA equivalence, without having
   * instantiated the other ISA. As such, it being uninstantiated does not allow
   * comparison of extensions.
   */
  bool eq(const ISAInfoBase *other, const QStringList &otherExts) const {
    const auto ext1 = QSet(this->enabledExtensions().begin(),
                           this->enabledExtensions().end());
    const auto ext2 = QSet(otherExts.begin(), otherExts.end());
    return this->name() == other->name() && ext1 == ext2;
  }

protected:
  ISAInfoBase() {}

  RegInfoSet m_regInfoSet;
};

struct ProcessorISAInfo {
  std::shared_ptr<ISAInfoBase> isa;
  QStringList supportedExtensions;
  QStringList defaultExtensions;
};

template <ISA isa>
class ISAInfo : public ISAInfoBase {};

} // namespace Ripes
