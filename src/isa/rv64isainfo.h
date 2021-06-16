#pragma once

#include "isainfo.h"
#include "rvisainfo_common.h"

namespace Ripes {

template <>
class ISAInfo<ISA::RV64I> : public ISAInfoBase {
public:
    ISAInfo<ISA::RV64I>(const QStringList extensions) {
        // Validate extensions
        for (const auto& ext : extensions) {
            if (supportsExtension(ext)) {
                m_enabledExtensions << ext;
            } else {
                assert(false && "Invalid extension specified for ISA");
            }
        }
    }

    QString name() const override { return "RV64I"; }
    ISA isaID() const override { return ISA::RV64I; }

    unsigned int regCnt() const override { return 32; }
    QString regName(unsigned i) const override {
        return RVISA::RegNames.size() > static_cast<int>(i) ? RVISA::RegNames.at(static_cast<int>(i)) : QString();
    }
    QString regAlias(unsigned i) const override {
        return RVISA::RegAliases.size() > static_cast<int>(i) ? RVISA::RegAliases.at(static_cast<int>(i)) : QString();
    }
    QString regInfo(unsigned i) const override {
        return RVISA::RegDescs.size() > static_cast<int>(i) ? RVISA::RegDescs.at(static_cast<int>(i)) : QString();
    }
    bool regIsReadOnly(unsigned i) const override { return i == 0; }
    unsigned int bits() const override { return 64; }
    int spReg() const override { return 2; }
    int gpReg() const override { return 3; }
    int syscallReg() const override { return 17; }
    unsigned elfMachineId() const override { return EM_RISCV; }
    unsigned int regNumber(const QString& reg, bool& success) const override {
        QString regRes = reg;
        success = true;
        if (reg[0] == 'x' && (RVISA::RegNames.count(reg) != 0)) {
            regRes.remove('x');
            return regRes.toInt(&success, 10);
        } else if (RVISA::RegAliases.contains(reg)) {
            return RVISA::RegAliases.indexOf(reg);
        }
        success = false;
        return 0;
    }

    QString CCmarch() const override {
        QString march = "rv64i";

        // Proceed in canonical order
        for (const auto& ext : {"M", "A", "F", "D"}) {
            if (m_enabledExtensions.contains(ext)) {
                march += QString(ext).toLower();
            }
        }

        return march;
    }
    QString CCmabi() const override { return "lp64"; }

    QString elfSupportsFlags(unsigned flags) const override {
        /** We expect no flags for RV32IM compiled RISC-V executables.
         *  Refer to: https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md#-elf-object-files
         */
        if (flags == 0)
            return QString();
        QString err;
        for (const auto& flag : RVABI::ELFFlagStrings) {
            if (flags & flag.first) {
                err += "ELF flag '" + RVABI::ELFFlagStrings.at(flag.first) + "' unsupported<br/>";
            }
        }
        return err;
    }

    const QStringList& supportedExtensions() const override { return m_supportedExtensions; }
    const QStringList& enabledExtensions() const override { return m_enabledExtensions; }

private:
    QStringList m_enabledExtensions;
    QStringList m_supportedExtensions = {"M"};
};

}  // namespace Ripes
