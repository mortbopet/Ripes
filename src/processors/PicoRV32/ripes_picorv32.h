#pragma once

#include "../../isa/rv32isainfo.h"
#include "../../isa/rvisainfo_common.h"
#include "../interface/ripesprocessor.h"

class Vpicorv32;

namespace Ripes {

/**
 * @brief The PicoRV32 class
 * RipesProcessor wrapper around the PicoRV32 core.
 */
class PicoRV32 : public RipesProcessor {
    // CPU state, copied from picorv32.v
    enum State {
        cpu_state_trap = 0b10000000,
        cpu_state_fetch = 0b01000000,
        cpu_state_ld_rs1 = 0b00100000,
        cpu_state_ld_rs2 = 0b00010000,
        cpu_state_exec = 0b00001000,
        cpu_state_shift = 0b00000100,
        cpu_state_stmem = 0b00000010,
        cpu_state_ldmem = 0b00000001
    };

    // Copied from picorv32.v
    std::map<State, QString> stateToString = {{cpu_state_trap, "trap"},     {cpu_state_fetch, "fetch"},
                                              {cpu_state_ld_rs1, "ld_rs1"}, {cpu_state_ld_rs2, "ld_rs2"},
                                              {cpu_state_exec, "exec"},     {cpu_state_shift, "shift"},
                                              {cpu_state_stmem, "stmem"},   {cpu_state_ldmem, "ldmem"}};

public:
    PicoRV32(const QStringList& /* extensions */);
    ~PicoRV32();

    void clockProcessor() override;
    void resetProcessor() override;
    unsigned features() const { return m_features; }
    static const ISAInfoBase* supportsISA() {
        static auto s_isa = ISAInfo<ISA::RV32I>({"M"});
        return &s_isa;
    }
    const ISAInfoBase* implementsISA() const override { return supportsISA(); }
    virtual const std::set<RegisterFileType> registerFiles() const override { return {RegisterFileType::GPR}; }
    unsigned int stageCount() const override { return 1; }
    unsigned int getPcForStage(unsigned) const override;
    QString stageName(unsigned) const override { return "*"; };
    AInt nextFetchedAddress() const override;
    StageInfo stageInfo(unsigned) const override;
    const std::vector<unsigned> breakpointTriggeringStages() const override { return {0}; };
    vsrtl::core::AddressSpaceMM& getMemory() override { return m_memory; };
    MemoryAccess dataMemAccess() const override { return m_dataAccess; }
    MemoryAccess instrMemAccess() const override { return m_instrAccess; }
    VInt getRegister(RegisterFileType, unsigned i) const override;
    void setRegister(RegisterFileType, unsigned i, VInt v) override;
    void setProgramCounter(AInt /* address */) override;
    void setPCInitialValue(AInt /* address */) override;
    void finalize(FinalizeReason finalizeReason) override;
    bool finished() const override { return m_finished; };
    long long getInstructionsRetired() const override;
    long long getCycleCount() const override;

private:
    PicoRV32(const PicoRV32&) = delete;
    PicoRV32& operator=(const PicoRV32&) = delete;

    void handleMemoryAccess();

    bool m_finished = false;
    bool m_doPCPI = false;
    Vpicorv32* top = nullptr;
    vsrtl::core::AddressSpaceMM m_memory;
    AInt m_initialPC = 0x0;

    MemoryAccess m_dataAccess;
    MemoryAccess m_instrAccess;
};

}  // namespace Ripes
