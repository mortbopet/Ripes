#include "rvisainfo_common.h"

namespace Ripes {
namespace RVISA {

// clang-format off
const QStringList RegAliases = QStringList() << "zero"
    << "ra" << "sp" << "gp" << "tp" << "t0" << "t1" << "t2" << "s0" << "s1" << "a0"
    << "a1" << "a2" << "a3" << "a4" << "a5" << "a6" << "a7" << "s2" << "s3" << "s4"
    << "s5" << "s6" << "s7" << "s8" << "s9" << "s10" << "s11" << "t3" << "t4" << "t5"
    << "t6";

const QStringList RegNames = QStringList() << "x0"
    << "x1" << "x2" << "x3" << "x4" << "x5" << "x6" << "x7" << "x8"
    << "x9" << "x10" << "x11" << "x12" << "x13" << "x14" << "x15"
    << "x16" << "x17" << "x18" << "x19" << "x20" << "x21" << "x22" << "x23"
    << "x24" << "x25" << "x26" << "x27" << "x28" << "x29" << "x30" << "x31";

const QStringList RegDescs = QStringList() << "Hard-Wired zero"
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
}  // namespace RVISA

namespace RVABI {
const std::map<RVElfFlags, QString> ELFFlagStrings{{RVC, "RVC"}, {FloatABI, "Float ABI"}, {RVE, "RVE"}, {TSO, "TSO"}};
}

}  // namespace Ripes
