#pragma once

#include <QMap>
#include <QSet>
#include <QString>
#include <set>

#include "elfio/elf_types.hpp"

namespace Ripes {

/// List of currently supported ISAs
enum class ISA { RV32I, RV64I };
const static std::map<ISA, QString> ISAFamilyNames = {{ISA::RV32I, "RISC-V"}, {ISA::RV64I, "RISC-V"}};
enum class RegisterFileType { GPR, FPR, CSR };
const static std::map<RegisterFileType, QString> s_RegsterFileName = {{RegisterFileType::GPR, "GPR"},
                                                                      {RegisterFileType::FPR, "FPR"},
                                                                      {RegisterFileType::CSR, "CSR"}};

class ISAInfoBase {
public:
    virtual ~ISAInfoBase(){};
    virtual QString name() const = 0;
    virtual ISA isaID() const = 0;

    virtual unsigned regCnt() const = 0;
    virtual QString regName(unsigned i) const = 0;
    virtual unsigned regNumber(const QString& regName, bool& success) const = 0;
    virtual QString regAlias(unsigned i) const = 0;
    virtual QString regInfo(unsigned i) const = 0;
    virtual bool regIsReadOnly(unsigned i) const = 0;
    virtual unsigned bits() const = 0;                              // Register width, in bits
    unsigned bytes() const { return bits() / CHAR_BIT; }            // Register width, in bytes
    virtual unsigned instrBits() const = 0;                         // Instruction width, in bits
    unsigned instrBytes() const { return instrBits() / CHAR_BIT; }  // Instruction width, in bytes
    virtual int spReg() const { return -1; }                        // Stack pointer
    virtual int gpReg() const { return -1; }                        // Global pointer
    virtual int syscallReg() const { return -1; }                   // Syscall function register

    // GCC Compile command architecture and ABI specification strings
    virtual QString CCmarch() const = 0;
    virtual QString CCmabi() const = 0;

    virtual unsigned elfMachineId() const = 0;

    /**
     * @brief elfSupportsFlags
     * The instructcion set should determine whether the provided @p flags, as retrieved from an ELF file, are valid
     * flags for the instruction set. If a mismatch is found, an error message describing the
     * mismatch is returned. Else, returns an empty QString(), validating the flags.
     */
    virtual QString elfSupportsFlags(unsigned flags) const = 0;

    /**
     * @brief supportedExtensions/enabledExtensions
     * An ISA may have a set of (optional) extensions which may be enabled/disabled for a given processor.
     * SupportedExtensions can be used when ie. instantiating a processor and enabledExtensions when instantiating an
     * assembler for a given processor.
     */
    virtual const QStringList& supportedExtensions() const = 0;
    virtual const QStringList& enabledExtensions() const = 0;
    bool extensionEnabled(const QString& ext) const { return enabledExtensions().contains(ext); }
    bool supportsExtension(const QString& ext) const { return supportedExtensions().contains(ext); }

    /**
     * ISA equality is defined as a separate function rather than the == operator, given that we might need to check for
     * ISA equivalence, without having instantiated the other ISA. As such, it being uninstantiated does not allow
     * compoarison of extensions.
     */
    bool eq(const ISAInfoBase* other, const QStringList& otherExts) const {
        const auto ext1 = QSet(this->enabledExtensions().begin(), this->enabledExtensions().end());
        const auto ext2 = QSet(otherExts.begin(), otherExts.end());
        return this->name() == other->name() && ext1 == ext2;
    }

protected:
    ISAInfoBase() {}
};

template <ISA isa>
class ISAInfo : public ISAInfoBase {};

}  // namespace Ripes
