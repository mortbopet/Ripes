#pragma once

#include <type_traits>

#include "processorhandler.h"
#include "ripes_syscall.h"
#include "systemio.h"

namespace Ripes {

template <typename BaseSyscall>
class ExitSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    ExitSyscall() : BaseSyscall("Exit", "Exits the program with code 0") {}
    void execute() {
        SystemIO::printString("\nProgram exited with code: 0");
        FinalizeReason fr;
        fr.exitSyscall = true;
        ProcessorHandler::get()->getProcessorNonConst()->finalize(fr);
    }
};

template <typename BaseSyscall>
class Exit2Syscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    Exit2Syscall() : BaseSyscall("Exit2", "Exits the program with a code", {{0, "the number to exit with"}}) {}
    void execute() {
        SystemIO::printString("\nProgram exited with code: " + QString::number(BaseSyscall::getArg(0)));
        FinalizeReason fr;
        fr.exitSyscall = true;
        ProcessorHandler::get()->getProcessorNonConst()->finalize(fr);
    }
};

}  // namespace Ripes
