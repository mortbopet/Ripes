#pragma once

#include "limits.h"
#include <QMap>
#include <QSet>
#include <QString>
#include <memory>
#include <set>
#include <vector>

#include "instruction.h"
#include "pseudoinstruction.h"

namespace Ripes {

/// List of currently supported ISAs.
enum class ISA { RV32I, RV64I, MIPS32I };
const static std::map<ISA, QString> ISAFamilyNames = {
    {ISA::RV32I, "RISC-V"}, {ISA::RV64I, "RISC-V"}, {ISA::MIPS32I, "MIPS"}};
struct RegisterFileName {
  QString shortName;
  QString longName;
};

/// An interface into a register file.
struct RegFileInfoInterface {
  virtual ~RegFileInfoInterface(){};
  /// Returns this register file's type.
  virtual std::string_view regFileName() const = 0;
  /// Returns this register file's description.
  virtual std::string_view regFileDesc() const = 0;
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

/// An index into a single register.
struct RegIndex {
  const std::shared_ptr<const RegFileInfoInterface> file;
  const unsigned index;
};

using RegInfoVec = std::vector<std::shared_ptr<const RegFileInfoInterface>>;
using RegInfoMap =
    std::map<std::string_view, std::shared_ptr<const RegFileInfoInterface>>;

/// The ISAInfoBase class defines an interface for instruction set information.
class ISAInfoBase {
public:
  virtual ~ISAInfoBase(){};
  virtual QString name() const = 0;
  virtual ISA isaID() const = 0;
  virtual const RegInfoMap &regInfoMap() const = 0;

  std::set<std::string_view> regFileNames() const {
    std::set<std::string_view> names;
    for (const auto &regInfo : regInfoMap()) {
      names.insert(regInfo.second->regFileName());
    }
    return names;
  }
  RegInfoVec regInfos() const {
    RegInfoVec regVec;
    for (const auto &regInfo : regInfoMap()) {
      regVec.emplace_back(regInfo.second);
    }
    return regVec;
  }
  std::optional<const RegFileInfoInterface *>
  regInfo(const std::string_view &regFileName) const {
    if (auto match = regInfoMap().find(regFileName);
        match != regInfoMap().end()) {
      return {regInfoMap().at(regFileName).get()};
    }
    return {};
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
  } // Instruction Alignment, in bytes
  virtual std::optional<RegIndex> spReg() const { return {}; } // Stack pointer
  virtual std::optional<RegIndex> gpReg() const { return {}; } // Global pointer
  virtual std::optional<RegIndex> syscallReg() const {
    return {};
  } // Syscall function register
  // Mapping between syscall argument # and the corresponding register # wherein
  // that argument is passed.
  virtual std::optional<RegIndex> syscallArgReg(unsigned /*argIdx*/) const {
    return {};
  }

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

  /// Returns the set of instructions for this ISA
  virtual const InstrVec &instructions() const = 0;
  /// Returns the set of pseudoinstructions for this ISA
  virtual const PseudoInstrVec &pseudoInstructions() const = 0;
  /// Returns the set of relocations for this ISA
  virtual const RelocationsVec &relocations() const = 0;

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
};

struct ProcessorISAInfo {
  std::shared_ptr<ISAInfoBase> isa;
  QStringList supportedExtensions;
  QStringList defaultExtensions;
};

template <ISA isa>
class ISAInfo : public ISAInfoBase {};

using ISAInfoMap =
    std::map<std::pair<ISA, QStringList>, std::shared_ptr<ISAInfoBase>>;

struct ISAInfoRegistry {
  template <ISA isa>
  static const std::shared_ptr<ISAInfoBase> &
  getISA(const QStringList &extensions) {
    return instance().supportedISA<isa>(extensions);
  }

  template <ISA isa>
  static const std::shared_ptr<ISAInfoBase> &getSupportedISA() {
    return instance().supportedISA<isa>();
  }

private:
  static ISAInfoRegistry &instance() {
    static ISAInfoRegistry r;
    return r;
  }

  template <ISA isa>
  const std::shared_ptr<ISAInfoBase> &supportedISA(
      const QStringList &extensions = ISAInfo<isa>::getSupportedExtensions()) {
    auto key = std::pair(isa, extensions);
    if (supportedISAMap.count(key) == 0) {
      supportedISAMap[key] = std::make_shared<ISAInfo<isa>>(extensions);
    }
    return supportedISAMap.at(key);
  }

  ISAInfoMap supportedISAMap;
};

} // namespace Ripes
