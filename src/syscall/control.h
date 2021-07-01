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
        ProcessorHandler::getProcessorNonConst()->finalize(RipesProcessor::FinalizeReason::exitSyscall);
    }
};

template <typename BaseSyscall>
class Exit2Syscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    Exit2Syscall() : BaseSyscall("Exit2", "Exits the program with a code", {{0, "the number to exit with"}}) {}
    void execute() {
        SystemIO::printString("\nProgram exited with code: " +
                              QString::number(BaseSyscall::getArg(RegisterFileType::GPR, 0)));
        ProcessorHandler::getProcessorNonConst()->finalize(RipesProcessor::FinalizeReason::exitSyscall);
    }
};

template <typename BaseSyscall>
class BrkSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    BrkSyscall()
        : BaseSyscall(
              "brk",
              "Change the location of the program break, which defines the end of the process's data segment (i.e., "
              "the program break is the first location after the end of the uninitialized data segment).",
              {{0, "sets the end of the data segment to the specified address"}},
              {{0, "On success, brk() returns zero.  On error, -1 is returned, and errno is set to ENOMEM"}}) {}

    void execute() {
        // Nothing to do - at the moment we allow infinite growth of the heap.
        // @todo: Add checks to ensure that the heap doesn't grow into the stack segment...!
        BaseSyscall::setRet(RegisterFileType::GPR, 0, 0);
    }
};

}  // namespace Ripes
