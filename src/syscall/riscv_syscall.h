#pragma once

#include "processorhandler.h"
#include "ripes_syscall.h"

// Syscall headers
#include "control.h"
#include "file.h"
#include "print.h"
#include "syscall_time.h"

namespace Ripes {

class RISCVSyscall : public Syscall {
public:
    RISCVSyscall(const QString& name, const QString& description = QString(),
                 const std::map<ArgIdx, QString>& argumentDescriptions = std::map<ArgIdx, QString>(),
                 const std::map<ArgIdx, QString>& returnDescriptions = std::map<ArgIdx, QString>())
        : Syscall(name, description, argumentDescriptions, returnDescriptions) {}
    ~RISCVSyscall() override {}

    VInt getArg(RegisterFileType rfid, ArgIdx i) const override {
        // RISC-V arguments range from a0-a6
        assert(i < 7);
        const int regIdx = 10 + i;  // a0 = x10
        return ProcessorHandler::getRegisterValue(rfid, regIdx);
    }

    void setRet(RegisterFileType rfid, ArgIdx i, VInt value) const override {
        // RISC-V arguments range from a0-a6
        assert(i < 7);
        const int regIdx = 10 + i;  // a0 = x10
        ProcessorHandler::setRegisterValue(rfid, regIdx, value);
    }
};

class RISCVSyscallManager : public SyscallManagerT<RISCVSyscall> {
public:
    RISCVSyscallManager() {
        // Print syscalls
        emplace<PrintIntSyscall<RISCVSyscall>>(RVABI::PrintInt);
        emplace<PrintFloatSyscall<RISCVSyscall>>(RVABI::PrintFloat);
        emplace<PrintStrSyscall<RISCVSyscall>>(RVABI::PrintStr);
        emplace<PrintCharSyscall<RISCVSyscall>>(RVABI::PrintChar);
        emplace<PrintHexSyscall<RISCVSyscall>>(RVABI::PrintIntHex);
        emplace<PrintBinarySyscall<RISCVSyscall>>(RVABI::PrintIntBinary);
        emplace<PrintUnsignedSyscall<RISCVSyscall>>(RVABI::PrintIntUnsigned);

        // Control syscalls
        emplace<ExitSyscall<RISCVSyscall>>(RVABI::Exit);
        emplace<Exit2Syscall<RISCVSyscall>>(RVABI::Exit2);
        emplace<BrkSyscall<RISCVSyscall>>(RVABI::brk);

        // File syscalls
        emplace<CloseSyscall<RISCVSyscall>>(RVABI::Close);
        emplace<LSeekSyscall<RISCVSyscall>>(RVABI::LSeek);
        emplace<ReadSyscall<RISCVSyscall>>(RVABI::Read);
        emplace<OpenSyscall<RISCVSyscall>>(RVABI::Open);
        emplace<WriteSyscall<RISCVSyscall>>(RVABI::Write);
        emplace<GetCWDSyscall<RISCVSyscall>>(RVABI::GetCWD);
        emplace<FStatSyscall<RISCVSyscall>>(RVABI::FStat);

        // Time syscalls
        emplace<CyclesSyscall<RISCVSyscall>>(RVABI::Cycles);
        emplace<TimeMsSyscall<RISCVSyscall>>(RVABI::TimeMs);
    }
};

}  // namespace Ripes
