#pragma once

#include <type_traits>

#include "processorhandler.h"
#include "ripes_syscall.h"
#include "systemio.h"

#include <QDateTime>

namespace Ripes {
template <typename BaseSyscall>
class CyclesSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    CyclesSyscall()
        : BaseSyscall("Cycles", "Get number of cycles elapsed since program start", {},
                      {{0, "low 32 bits of cycles elapsed"}, {1, "high 32 bits of cycles elapsed"}}) {}
    void execute() {
        static long long cycleCount = ProcessorHandler::get()->getProcessor()->getCycleCount();
        BaseSyscall::setRet(RegisterFileType::GPR, 0, cycleCount & 0xFFFFFFFF);
        BaseSyscall::setRet(RegisterFileType::GPR, 1, (cycleCount >> 32) & 0xFFFFFFFF);
    }
};

template <typename BaseSyscall>
class TimeMsSyscall : public BaseSyscall {
    static_assert(std::is_base_of<Syscall, BaseSyscall>::value);

public:
    TimeMsSyscall()
        : BaseSyscall(
              "Time_msec", "Get the current time since epoch (milliseconds since 1 January 1970)", {},
              {{0, "low 32 bits of milliseconds since epoch"}, {1, "high 32 bits of milliseconds since epoch"}}) {}
    void execute() {
        static long long ms = QDateTime::currentMSecsSinceEpoch();
        BaseSyscall::setRet(RegisterFileType::GPR, 0, ms & 0xFFFFFFFF);
        BaseSyscall::setRet(RegisterFileType::GPR, 1, (ms >> 32) & 0xFFFFFFFF);
    }
};

}  // namespace Ripes
