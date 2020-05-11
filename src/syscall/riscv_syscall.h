#pragma once

#include "processorhandler.h"
#include "ripes_syscall.h"

// Syscall headers
#include "control.h"
#include "print.h"

namespace Ripes {

class RISCVSyscall : public Syscall {
public:
    RISCVSyscall(const QString& name, const QString& description = QString(),
                 const std::map<unsigned, QString>& argumentDescriptions = std::map<unsigned, QString>())
        : Syscall(name, description, argumentDescriptions) {}
    ~RISCVSyscall() override {}
    uint32_t getArg(unsigned i) const override {
        // RISC-V arguments range from a0-a6
        assert(i < 7);
        const int regIdx = 10 + i;  // a0 = x10
        return ProcessorHandler::get()->getRegisterValue(regIdx);
    }
};

class RISCVSyscallManager : public SyscallManagerT<RISCVSyscall> {
public:
    RISCVSyscallManager() {
        // Print syscalls
        emplace<PrintIntSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::PrintInt);
        emplace<PrintFloatSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::PrintFloat);
        emplace<PrintStrSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::PrintStr);
        emplace<PrintCharSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::PrintChar);
        emplace<PrintHexSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::PrintIntHex);
        emplace<PrintBinarySyscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::PrintIntBinary);
        emplace<PrintUnsignedSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::PrintIntUnsigned);

        // Control syscalls
        emplace<ExitSyscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::Exit);
        emplace<Exit2Syscall<RISCVSyscall>>(ISAInfo<ISA::RV32IM>::Exit2);
    }
};

}  // namespace Ripes
