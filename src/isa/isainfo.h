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

  /// Returns Register width, in bits
  virtual unsigned bits() const = 0;
  /// Register width, in bytes
  unsigned bytes() const { return bits() / CHAR_BIT; }
};

typedef uint ExtensionID_t;
class ExtensionSetInfo;
/// Interface for an ISA extension, which may be supported by an ISA, and
/// enabled for a given processor.
class ExtensionInfoInterface {
public:
  virtual ~ExtensionInfoInterface(){};

  /// unique ID of the extension, used for extension lookup, comparison abd by
  /// default as key for canonical ordering.
  virtual ExtensionID_t id() const = 0;
  /// name of the extension, used for user-facing messages and extension lookup
  /// by name.
  virtual QString name() const = 0;
  /// description of the extension, used for user-facing messages.
  virtual QString description() const = 0;

  /// name used for the march specifier of the extension, used for compiler
  /// integration.
  virtual QString CCmarchName() const = 0;

  /// extensions that are implicitly enabled/required by this extension.
  virtual const ExtensionSetInfo &implicates() const = 0;

  /// Extensions are uniquely identified by their ID.
  /// Note: Extensions of different architectures shall not be compared.
  auto operator<=>(const ExtensionInfoInterface &other) const {
    return id() <=> other.id();
  }

  bool operator==(const ExtensionInfoInterface &other) const {
    return *this <=> other == 0;
  }
  bool operator!=(const ExtensionInfoInterface &other) const {
    return !(*this == other);
  }
};

struct ExtensionComparator {
  using is_transparent = void; // enables heterogeneous lookup in sets and maps
                               // using this comparator
  using ExtConstPtr = const ExtensionInfoInterface *;
  using ExtConstRef = const ExtensionInfoInterface &;

  bool operator()(ExtConstPtr a, ExtConstPtr b) const { return *a < *b; }
  bool operator()(ExtConstRef a, ExtConstPtr b) const { return a < *b; }
  bool operator()(ExtConstPtr a, ExtConstRef b) const { return *a < b; }
  bool operator()(ExtConstRef a, ExtConstRef b) const { return a < b; }
};

using ExtensionContainer_t =
    std::set<const ExtensionInfoInterface *, ExtensionComparator>;

/// An interface for a set of extensions of a common architecture
/// Note: Extensions of different architectures shall not be combined.
class ExtensionSetInfo {
public:
  using UniquePtr = std::unique_ptr<ExtensionSetInfo>;
  using Ptr = std::shared_ptr<ExtensionSetInfo>;
  using ConstPtr = std::shared_ptr<const ExtensionSetInfo>;

  virtual ~ExtensionSetInfo(){};

  /// Returns a set of extensions in canonical order, as defined by the ISA
  /// specification.
  virtual const ExtensionContainer_t &extensions() const = 0;

  /// Returns the compiler march string for the set of extensions, as defined by
  /// the ISA specification.
  virtual QString CCmarch() const = 0;

  QStringList toStringList() const {
    QStringList extNames;
    for (const auto *ext : extensions()) {
      extNames << ext->name();
    }
    return extNames;
  }

  /// Searches the extension of selected id recursively in set of
  /// extensions(+implicates). Returns nullopt if no matching extension is
  /// found.
  std::optional<const ExtensionInfoInterface *>
  getExtension(ExtensionID_t id) const {
    std::set<ExtensionID_t> visitedIDs;
    return getExtension(
        [id](const ExtensionInfoInterface *ext) { return ext->id() == id; },
        visitedIDs);
  }
  /// Searches the extension of selected name recursively in set of
  /// extensions(+implicates). Returns nullopt if no matching extension is
  /// found.
  std::optional<const ExtensionInfoInterface *>
  getExtension(const QString &extName) const {
    std::set<ExtensionID_t> visitedIDs;
    return getExtension(
        [&extName](const ExtensionInfoInterface *ext) {
          return ext->name() == extName;
        },
        visitedIDs);
  }
  /// Searches the selected extension recursively in set of
  /// extensions(+implicates). Returns nullopt if no matching extension is
  /// found.
  std::optional<const ExtensionInfoInterface *>
  getExtension(const ExtensionInfoInterface &ext) const {
    std::set<ExtensionID_t> visitedIDs;
    return getExtension(
        [&ext](const ExtensionInfoInterface *e) { return (*e) == ext; },
        visitedIDs);
  }

  template <typename ID_t = ExtensionID_t>
  bool containsExtension(ID_t id) const {
    return getExtension(static_cast<ExtensionID_t>(id)).has_value();
  }
  bool containsExtension(const QString &ext) const {
    return getExtension(ext).has_value();
  }
  bool containsExtension(const ExtensionInfoInterface &ext) const {
    return getExtension(ext).has_value();
  }

  /// Returns true if this extension set is a subset of the super set, i.e. all
  /// extensions in this set are also present in the super set.
  bool isSubsetOf(const ExtensionSetInfo &super) const {
    for (const ExtensionInfoInterface *ext : extensions()) {
      if (!super.containsExtension(*ext)) {
        return false;
      }
    }
    return true;
  }

  bool operator==(const ExtensionSetInfo &other) const {
    const auto &exts1 = extensions();
    const auto &exts2 = other.extensions();

    if (exts1.size() != exts2.size()) {
      return false;
    }

    for (const auto *ext : exts1) {
      if (!other.containsExtension(*ext)) {
        return false;
      }
    }

    return true;
  }
  bool operator!=(const ExtensionSetInfo &other) const = default;

  virtual UniquePtr clone() const = 0;
  virtual ExtensionSetInfo &operator<<(const ExtensionInfoInterface &ext) = 0;
  ExtensionSetInfo &operator<<(const ExtensionSetInfo &extinfo) {
    for (const auto *ext : extinfo.extensions()) {
      (*this) << *ext;
    }
    return *this;
  };
  virtual const ExtensionInfoInterface *
  remove(const ExtensionInfoInterface &ext) = 0;

private:
  std::optional<const ExtensionInfoInterface *>
  getExtension(std::function<bool(const ExtensionInfoInterface *)> predicate,
               std::set<ExtensionID_t> &visitedIDs) const {
    for (const auto &ext : extensions()) {
      if (visitedIDs.contains(ext->id())) {
        continue;
      }

      visitedIDs.insert(ext->id());

      if (predicate(ext)) {
        return ext;
      }

      auto foundExt = ext->implicates().getExtension(predicate, visitedIDs);
      if (foundExt.has_value()) {
        return foundExt;
      }
    }

    return std::nullopt;
  }
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
  /// get march string with the currently enabled extensions
  QString CCmarch(void) const { return CCmarch(enabledExtensions()); }
  /// get march string with the provided extensions
  virtual QString CCmarch(const ExtensionSetInfo &extensions) const = 0;
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
  virtual const ExtensionSetInfo &supportedExtensions() const = 0;
  virtual const ExtensionSetInfo &enabledExtensions() const = 0;
  bool extensionEnabled(const ExtensionInfoInterface &ext) const {
    return enabledExtensions().containsExtension(ext);
  }
  bool supportsExtension(const ExtensionInfoInterface &ext) const {
    return supportedExtensions().containsExtension(ext);
  }

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
  bool eq(const ISAInfoBase *other, const ExtensionSetInfo &otherExts) const {
    return this->name() == other->name() &&
           this->enabledExtensions() == otherExts;
  }

protected:
  ISAInfoBase() {}
};

struct ProcessorISAInfo {
  std::shared_ptr<const ISAInfoBase> isa;
  ExtensionSetInfo::ConstPtr supportedExtensions;
  ExtensionSetInfo::ConstPtr defaultExtensions;
};

template <ISA isa>
class ISAInfo : public ISAInfoBase {};

struct ISAInfoRegistry {
  template <ISA isa>
  static const std::shared_ptr<ISAInfoBase> &
  getISA(const ExtensionSetInfo &extensions) {
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
  const std::shared_ptr<ISAInfoBase> &
  supportedISA(const ExtensionSetInfo &extensions =
                   ISAInfo<isa>::getSupportedExtensions()) {

    auto it = std::find_if(
        registeredISAs.begin(), registeredISAs.end(),
        [&extensions](const std::shared_ptr<ISAInfoBase> &isaInfo) {
          return isaInfo->isaID() == isa &&
                 isaInfo->enabledExtensions() == extensions;
        });

    if (it != registeredISAs.end()) {
      return *it;
    } else {
      // at this point, we enforce that all ISAInfos are constructable with an
      // argument of type const ExtensionSetInfo&
      registeredISAs.emplace_back(std::make_shared<ISAInfo<isa>>(extensions));
      return registeredISAs.back();
    }
  }

  std::vector<std::shared_ptr<ISAInfoBase>> registeredISAs;
};

} // namespace Ripes
