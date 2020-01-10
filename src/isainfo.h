#pragma once

#include <QMap>
#include <QString>

/// Currently supported ISAs
enum class ISA { RV32IM };

class ISAInfoBase {
public:
    virtual QString name() const = 0;

    virtual unsigned regCnt() const = 0;
    virtual QString regName(unsigned i) const = 0;
    virtual QString regAlias(unsigned i) const = 0;
    virtual QString regInfo(unsigned i) const = 0;
    virtual bool regIsReadOnly(unsigned i) const = 0;
    virtual unsigned bits() const = 0;
    unsigned bytes() const { return bits() / CHAR_BIT; }

protected:
    ISAInfoBase() {}
};

template <ISA isa>
class ISAInfo : public ISAInfoBase {
public:
    static ISAInfo<isa>& instance();

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
// clang-format on

}  // namespace

template <>
class ISAInfo<ISA::RV32IM> : public ISAInfoBase {
public:
    static ISAInfo<ISA::RV32IM>& instance() {
        static ISAInfo<ISA::RV32IM> pr;
        return pr;
    }

    QString name() const { return "RV32IM"; }

    unsigned int regCnt() const override { return 32; }
    QString regName(unsigned i) const override { return RVRegNames.at(i); }
    QString regAlias(unsigned i) const override { return RVRegAliases.at(i); }
    QString regInfo(unsigned i) const override { return RVRegDescs.at(i); }
    bool regIsReadOnly(unsigned i) const override { return i == 0; }
    unsigned int bits() const override { return 32; }
};
