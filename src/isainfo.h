#pragma once

#include <QMap>
#include <QString>

#include "elfio/elf_types.hpp"

namespace Ripes {

/// Currently supported ISAs
enum class ISA { RV32IM };
const static std::map<ISA, QString> ISANames = {{ISA::RV32IM, "RISC-V"}};

class ISAInfoBase {
public:
    virtual QString name() const = 0;
    virtual ISA isaID() const = 0;

    virtual unsigned regCnt() const = 0;
    virtual QString regName(unsigned i) const = 0;
    virtual QString regAlias(unsigned i) const = 0;
    virtual QString regInfo(unsigned i) const = 0;
    virtual bool regIsReadOnly(unsigned i) const = 0;
    virtual unsigned bits() const = 0;
    unsigned bytes() const { return bits() / CHAR_BIT; }
    virtual int spReg() const { return -1; }       // Stack pointer
    virtual int gpReg() const { return -1; }       // Global pointer
    virtual int syscallReg() const { return -1; }  // Syscall function register

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

protected:
    ISAInfoBase() {}
};

template <ISA isa>
class ISAInfo : public ISAInfoBase {
public:
    static const ISAInfo<isa>* instance();

private:
    ISAInfo<isa>() {}
};

// ==================================== RISCV ====================================
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
class ISAInfo<ISA::RV32IM> : public ISAInfoBase {
public:
    enum SysCall {
        None = 0,
        PrintInt = 1,
        PrintFloat = 2,
        PrintStr = 4,
        Exit = 10,
        PrintChar = 11,
        GetCWD = 17,
        PrintIntHex = 34,
        PrintIntBinary = 35,
        PrintIntUnsigned = 36,
        Close = 57,
        LSeek = 62,
        Read = 63,
        Write = 64,
        FStat = 80,
        Exit2 = 93,
        Open = 1024
    };
    static const ISAInfo<ISA::RV32IM>* instance() {
        static ISAInfo<ISA::RV32IM> pr;
        return &pr;
    }

    QString name() const override { return "RV32IM"; }
    ISA isaID() const override { return ISA::RV32IM; }

    unsigned int regCnt() const override { return 32; }
    QString regName(unsigned i) const override { return RVRegNames.at(i); }
    QString regAlias(unsigned i) const override { return RVRegAliases.at(i); }
    QString regInfo(unsigned i) const override { return RVRegDescs.at(i); }
    bool regIsReadOnly(unsigned i) const override { return i == 0; }
    unsigned int bits() const override { return 32; }
    int spReg() const override { return 2; }
    int gpReg() const override { return 3; }
    int syscallReg() const override { return 17; }
    unsigned elfMachineId() const override { return EM_RISCV; }

    QString CCmarch() const override { return "rv32im"; }
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
};

}  // namespace Ripes
