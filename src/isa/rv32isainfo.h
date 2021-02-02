#pragma once

#include "isainfo.h"

namespace Ripes {

namespace {
// clang-format off
const static QStringList RVRegAliases = QStringList() << "zero"
    << "ra" << "sp" << "gp" << "tp" << "t0" << "t1" << "t2" << "s0" << "s1" << "a0"
    << "a1" << "a2" << "a3" << "a4" << "a5" << "a6" << "a7" << "s2" << "s3" << "s4"
    << "s5" << "s6" << "s7" << "s8" << "s9" << "s10" << "s11" << "t3" << "t4" << "t5"
    << "t6";

const static QStringList RVRegNames = QStringList() << "x0"
    << "x1" << "x2" << "x3" << "x4" << "x5" << "x6" << "x7" << "x8"
    << "x9" << "x10" << "x11" << "x12" << "x13" << "x14" << "x15"
    << "x16" << "x17" << "x18" << "x19" << "x20" << "x21" << "x22" << "x23"
    << "x24" << "x25" << "x26" << "x27" << "x28" << "x29" << "x30" << "x31";

const static QStringList RVRegDescs = QStringList() << "Hard-Wired zero"
                                         << "Return Address \nSaver: Caller"
                                         << "Stack pointer\nSaver: Callee"
                                         << "Global pointer"
                                         << "Thread pointer"
                                         << "Temporary/alternate link register\nSaver: Caller"
                                         << "Temporary\nSaver: Caller"
                                         << "Temporary\nSaver: Caller"
                                         << "Saved register/frame pointer\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Function argument/return value\nSaver: Caller"
                                         << "Function argument/return value\nSaver: Caller"
                                         << "Function argument\nSaver: Caller"
                                         << "Function argument\nSaver: Caller"
                                         << "Function argument\nSaver: Caller"
                                         << "Function argument\nSaver: Caller"
                                         << "Function argument\nSaver: Caller"
                                         << "Function argument\nSaver: Caller"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Saved register\nSaver: Callee"
                                         << "Temporary register\nSaver: Caller"
                                         << "Temporary register\nSaver: Caller"
                                         << "Temporary register\nSaver: Caller"
                                         << "Temporary register\nSaver: Caller";
// RISC-V ELF info

// Elf flag masks
enum RVElfFlags {
    RVC = 0b1,
    FloatABI = 0b110,
    RVE = 0b1000,
    TSO = 0b10000
};

const static std::map<RVElfFlags, QString> RVELFFlagStrings {{RVC, "RVC"}, {FloatABI, "Float ABI"}, {RVE, "RVE"}, {TSO, "TSO"}};

// clang-format on

}  // namespace

template <>
class ISAInfo<ISA::RV32I> : public ISAInfoBase {
public:
    ISAInfo<ISA::RV32I>(const QStringList extensions) {
        // Validate extensions
        for (const auto& ext : extensions) {
            if (supportsExtension(ext)) {
                m_enabledExtensions << ext;
            } else {
                assert(false && "Invalid extension specified for ISA");
            }
        }
    }

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

    QString name() const override { return "RV32I"; }
    ISA isaID() const override { return ISA::RV32I; }

    unsigned int regCnt() const override { return 32; }
    QString regName(unsigned i) const override {
        return RVRegNames.size() > static_cast<int>(i) ? RVRegNames.at(static_cast<int>(i)) : QString();
    }
    QString regAlias(unsigned i) const override {
        return RVRegAliases.size() > static_cast<int>(i) ? RVRegAliases.at(static_cast<int>(i)) : QString();
    }
    QString regInfo(unsigned i) const override {
        return RVRegDescs.size() > static_cast<int>(i) ? RVRegDescs.at(static_cast<int>(i)) : QString();
    }
    bool regIsReadOnly(unsigned i) const override { return i == 0; }
    unsigned int bits() const override { return 32; }
    int spReg() const override { return 2; }
    int gpReg() const override { return 3; }
    int syscallReg() const override { return 17; }
    unsigned elfMachineId() const override { return EM_RISCV; }
    unsigned int regNumber(const QString& reg, bool& success) const override {
        QString regRes = reg;
        success = true;
        if (reg[0] == 'x' && (RVRegNames.count(reg) != 0)) {
            regRes.remove('x');
            return regRes.toInt(&success, 10);
        } else if (RVRegAliases.contains(reg)) {
            return RVRegAliases.indexOf(reg);
        }
        success = false;
        return 0;
    }

    QString CCmarch() const override {
        QString march = "rv32i";

        // Proceed in canonical order
        for (const auto& ext : {"M", "A", "F", "D"}) {
            if (m_enabledExtensions.contains(ext)) {
                march += QString(ext).toLower();
            }
        }

        return march;
    }
    QString CCmabi() const override { return "ilp32"; }

    QString elfSupportsFlags(unsigned flags) const override {
        /** We expect no flags for RV32IM compiled RISC-V executables.
         *  Refer to: https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md#-elf-object-files
         */
        if (flags == 0)
            return QString();
        QString err;
        for (const auto& flag : RVELFFlagStrings) {
            if (flags & flag.first) {
                err += "ELF flag '" + RVELFFlagStrings.at(flag.first) + "' unsupported<br/>";
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
