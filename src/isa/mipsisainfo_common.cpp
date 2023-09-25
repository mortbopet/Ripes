#include "mipsisainfo_common.h"

namespace Ripes {
namespace MIPSISA {

// clang-format off
const QStringList RegAliases = QStringList() << "zero"
                                             << "at" << "v0" << "v1" << "a0" << "a1" << "a2" << "a3" << "t0" << "t1" << "t2"
                                             << "t3" << "t4" << "t5" << "t6" << "t7" << "s0" << "s1" << "s2" << "s3" << "s4"
                                             << "s5" << "s6" << "s7" << "t8" << "t9" << "k0" << "k1" << "gp" << "sp" << "fp"
                                             << "ra" << "hi" << "lo";

const QStringList RegNames = QStringList() << "$0"
                                           << "$1" << "$2" << "$3" << "$4" << "$5" << "$6" << "$7" << "$8"
                                           << "$9" << "$10" << "$11" << "$12" << "$13" << "$14" << "$15"
                                           << "$16" << "$17" << "$18" << "$19" << "$20" << "$21" << "$22" << "$23"
                                           << "$24" << "$25" << "$26" << "$27" << "$28" << "$29" << "$30" << "$31" << "$32" << "$33";

const QStringList RegDescs = QStringList() << "Hard-Wired zero"
                                           << "Assembler Temporary"
                                           << "Return value from function call"
                                           << "Return value from function call"
                                           << "Argument 1"
                                           << "Argument 2"
                                           << "Argument 3"
                                           << "Argument 4"
                                           << "Temporary\nNot preserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Saved Temporary\nPreserved across call"
                                           << "Saved Temporary\nPreserved across call"
                                           << "Saved Temporary\nPreserved across call"
                                           << "Saved Temporary\nPreserved across call"
                                           << "Saved Temporary\nPreserved across call"
                                           << "Saved Temporary\nPreserved across call"
                                           << "Saved Temporary\nPreserved across call"
                                           << "Saved Temporary\nPreserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Temporary\nNot preserved across call"
                                           << "Kernel use register"
                                           << "Kernel use register"
                                           << "Global pointer"
                                           << "Stack pointer"
                                           << "Frame pointer"
                                           << "Return Address"
                                           << "Multiplication/Division register"
                                           << "Multiplication/Division register";
// clang-format on
} // namespace MIPSISA

namespace MIPSABI {
const std::map<MIPSElfFlags, QString> ELFFlagStrings{
    {NOREORDER, "NOREORDER"}, {PIC, "PIC"}, {CPIC, "CPIC"}, {ARCH, "ARCH"}};
}

} // namespace Ripes
