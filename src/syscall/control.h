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
  void execute() override {
    SystemIO::printString("\nProgram exited with code: 0\n");
    ProcessorHandler::getProcessorNonConst()->finalize(
        RipesProcessor::FinalizeReason::exitSyscall);
  }
};

template <typename BaseSyscall>
class Exit2Syscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  Exit2Syscall()
      : BaseSyscall("Exit2", "Exits the program with a code",
                    {{0, "the number to exit with"}}) {}
  void execute() override {
    SystemIO::printString(
        "\nProgram exited with code: " +
        QString::number(BaseSyscall::getArg(BaseSyscall::REG_FILE, 0)) + "\n");
    ProcessorHandler::getProcessorNonConst()->finalize(
        RipesProcessor::FinalizeReason::exitSyscall);
  }
};

template <typename BaseSyscall>
class BrkSyscall : public BaseSyscall {
  static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
  BrkSyscall()
      : BaseSyscall(
            "brk",
            "Change the location of the program break, which defines the end "
            "of the process's data segment (i.e., "
            "the program break is the first location after the end of the "
            "uninitialized data segment).",
            {{0, "the requested end of the data segment, or 0 to query the "
                 "current program break"}},
            {{0, "the new program break on success, otherwise the current "
                 "program break"}}) {}

  void execute() override {
    // Follows the semantics of the Linux brk syscall (as used by e.g. newlib's
    // _sbrk). The initial program break is the end of the loaded program.
    if (!m_break.has_value()) {
      auto program = ProcessorHandler::getProgram();
      constexpr AInt alignment = 16;
      const AInt dataEnd = program ? program->getDataEnd() : 0;
      m_initialBreak = (dataEnd + alignment - 1) & ~(alignment - 1);
      m_break = m_initialBreak;
    }

    const AInt newBreak = BaseSyscall::getArg(BaseSyscall::REG_FILE, 0);
    // Retrieve the current stack pointer from the processor handler
    const AInt stackPointer =
        ProcessorHandler::getProcessor()->getRegister(BaseSyscall::REG_FILE, 2);
    if (newBreak >= m_initialBreak) {
      if (newBreak >= stackPointer) {
        SystemIO::printString(
            "Error: Attempted to allocate memory overlapping stack segment\n");
      } else {
        m_break = newBreak;
      }
    }

    BaseSyscall::setRet(BaseSyscall::REG_FILE, 0, *m_break);
  }

  void reset() override { m_break.reset(); }

private:
  AInt m_initialBreak = 0;
  std::optional<AInt> m_break;
};

} // namespace Ripes
