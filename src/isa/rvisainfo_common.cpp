#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

// clang-format off
const QStringList GPRRegAliases = QStringList() << "zero"
                                             << "ra" << "sp" << "gp" << "tp" << "t0" << "t1" << "t2" << "s0" << "s1" << "a0"
                                             << "a1" << "a2" << "a3" << "a4" << "a5" << "a6" << "a7" << "s2" << "s3" << "s4"
                                             << "s5" << "s6" << "s7" << "s8" << "s9" << "s10" << "s11" << "t3" << "t4" << "t5"
                                             << "t6";

const QStringList GPRRegNames = QStringList() << "x0"
                                           << "x1" << "x2" << "x3" << "x4" << "x5" << "x6" << "x7" << "x8"
                                           << "x9" << "x10" << "x11" << "x12" << "x13" << "x14" << "x15"
                                           << "x16" << "x17" << "x18" << "x19" << "x20" << "x21" << "x22" << "x23"
                                           << "x24" << "x25" << "x26" << "x27" << "x28" << "x29" << "x30" << "x31";

const QStringList GPRRegDescs = QStringList() << "Hard-Wired zero"
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


const QStringList FPRRegAliases = QStringList() << "ft0" << "ft1" << "ft2" << "ft3" << "ft4" << "ft5" << "ft6" << "ft7" 
                                                << "fs0" << "fs1" 
                                                << "fa0" << "fa1" 
                                                << "fa2" << "fa3" << "fa4" << "fa5" << "fa6" << "fa7" 
                                                << "fs2" << "fs3" << "fs4" << "fs5" << "fs6" << "fs7" << "fs8" << "fs9" << "fs10" << "fs11" 
                                                << "ft8" << "ft9" << "ft10" << "ft11";

const QStringList FPRRegNames = QStringList()   << "f0"  << "f1"  << "f2"  << "f3"  << "f4"  << "f5"  << "f6"  << "f7" 
                                                << "f8"  << "f9"  << "f10" << "f11" << "f12" << "f13" << "f14" << "f15" 
                                                << "f16" << "f17" << "f18" << "f19" << "f20" << "f21" << "f22" << "f23" 
                                                << "f24" << "f25" << "f26" << "f27" << "f28" << "f29" << "f30" << "f31";

const QStringList FPRRegDescs = QStringList()   << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"

                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"

                                                << "FP argument/return value\nSaver: Caller"
                                                << "FP argument/return value\nSaver: Caller"

                                                << "FP argument\nSaver: Caller"
                                                << "FP argument\nSaver: Caller"
                                                << "FP argument\nSaver: Caller"
                                                << "FP argument\nSaver: Caller"
                                                << "FP argument\nSaver: Caller"
                                                << "FP argument\nSaver: Caller"

                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"
                                                << "FP Saved register\nSaver: Callee"

                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller"
                                                << "FP Temporary\nSaver: Caller";
#define CSR_REG(name) static_cast<unsigned>(RV_CSRInfo::CSR::name)
const QHash<unsigned, QString> CSRRegAliases = {
    {CSR_REG(FFLAGS), "fflags"},
    {CSR_REG(FRM),    "frm"},
    {CSR_REG(FCSR),   "fcsr"}
};
const QHash<unsigned, QString> CSRRegNames = {
    {CSR_REG(FFLAGS), "fflags"},
    {CSR_REG(FRM),    "frm"},
    {CSR_REG(FCSR),   "fcsr"}
};
const QHash<unsigned, QString> CSRRegDescs = {
    {CSR_REG(FFLAGS), "Floating-Point Accrued Exceptions"},
    {CSR_REG(FRM),    "Floating-Point Dynamic Rounding Mode"},
    {CSR_REG(FCSR),   "Floating-Point Control and Status Register (frm +fflags)"}
};
#undef CSR_REG
// clang-format on

} // namespace RVISA

namespace RVABI {
const std::map<RVElfFlags, QString> ELFFlagStrings{
    {RVC, "RVC"}, {FloatABI, "Float ABI"}, {RVE, "RVE"}, {TSO, "TSO"}};
}

} // namespace Ripes
