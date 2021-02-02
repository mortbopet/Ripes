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
                 const std::map<unsigned, QString>& argumentDescriptions = std::map<unsigned, QString>(),
                 const std::map<unsigned, QString>& returnDescriptions = std::map<unsigned, QString>())
        : Syscall(name, description, argumentDescriptions, returnDescriptions) {}
    ~RISCVSyscall() override {}

    uint32_t getArg(RegisterFileType rfid, unsigned i) const override {
        // RISC-V arguments range from a0-a6
        assert(i < 7);
        const int regIdx = 10 + i;  // a0 = x10
        return ProcessorHandler::get()->getRegisterValue(rfid, regIdx);
    }

    void setRet(RegisterFileType rfid, unsigned i, uint32_t value) const override {
        // RISC-V arguments range from a0-a6
        assert(i < 7);
        const int regIdx = 10 + i;  // a0 = x10
        ProcessorHandler::get()->setRegisterValue(rfid, regIdx, value);
    }
};

class RISCVSyscallManager : public SyscallManagerT<RISCVSyscall> {
public:
    RISCVSyscallManager() {
        // Print syscalls
        emplace<PrintIntSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::PrintInt);
        emplace<PrintFloatSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::PrintFloat);
        emplace<PrintStrSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::PrintStr);
        emplace<PrintCharSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::PrintChar);
        emplace<PrintHexSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::PrintIntHex);
        emplace<PrintBinarySyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::PrintIntBinary);
        emplace<PrintUnsignedSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::PrintIntUnsigned);

        // Control syscalls
        emplace<ExitSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::Exit);
        emplace<Exit2Syscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::Exit2);
        emplace<BrkSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::brk);

        // File syscalls
        emplace<CloseSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::Close);
        emplace<LSeekSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::LSeek);
        emplace<ReadSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::Read);
        emplace<OpenSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::Open);
        emplace<WriteSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::Write);
        emplace<GetCWDSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::GetCWD);
        emplace<FStatSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::FStat);

        // Time syscalls
        emplace<CyclesSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::Cycles);
        emplace<TimeMsSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32I>::TimeMs);
    }
};

}  // namespace Ripes
