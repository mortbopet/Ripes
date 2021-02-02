#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "../../ripesprocessor.h"

// Functional units
#include "../riscv.h"
#include "../rv_alu.h"
#include "../rv_branch.h"
#include "../rv_control.h"
#include "../rv_decode.h"
#include "../rv_ecallchecker.h"
#include "../rv_immediate.h"
#include "../rv_memory.h"
#include "../rv_registerfile.h"

// Stage separating registers
#include "rv5s_no_fw_hz_exmem.h"
#include "rv5s_no_fw_hz_idex.h"
#include "rv5s_no_fw_hz_ifid.h"
#include "rv5s_no_fw_hz_memwb.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class RV5S_NO_FW_HZ : public RipesProcessor {
public:
    enum Stage { IF = 0, ID = 1, EX = 2, MEM = 3, WB = 4, STAGECOUNT };
    RV5S_NO_FW_HZ(const QStringList& extensions)
        : RipesProcessor("5-Stage RISC-V Processor without forwarding or hazard detection") {
        m_enabledISA = std::make_shared<ISAInfo<ISA::RV32I>>(extensions);
        decode->setISA(m_enabledISA);

        // -----------------------------------------------------------------------
        // Program counter
        pc_reg->out >> pc_4->op1;
        4 >> pc_4->op2;
        pc_src->out >> pc_reg->in;

        // Note: pc_src works uses the PcSrc enum, but is selected by the boolean signal
        // from the controlflow OR gate. PcSrc enum values must adhere to the boolean
        // 0/1 values.
        controlflow_or->out >> pc_src->select;

        controlflow_or->out >> *efsc_or->in[0];
        ecallChecker->syscallExit >> *efsc_or->in[1];

        // -----------------------------------------------------------------------
        // Instruction memory
        pc_reg->out >> instr_mem->addr;
        instr_mem->setMemory(m_memory);

        // -----------------------------------------------------------------------
        // Decode
        ifid_reg->instr_out >> decode->instr;

        // -----------------------------------------------------------------------
        // Control signals
        decode->opcode >> control->opcode;

        // -----------------------------------------------------------------------
        // Immediate
        decode->opcode >> immediate->opcode;
        ifid_reg->instr_out >> immediate->instr;

        // -----------------------------------------------------------------------
        // Registers
        decode->r1_reg_idx >> registerFile->r1_addr;
        decode->r2_reg_idx >> registerFile->r2_addr;
        reg_wr_src->out >> registerFile->data_in;

        memwb_reg->wr_reg_idx_out >> registerFile->wr_addr;
        memwb_reg->reg_do_write_out >> registerFile->wr_en;
        memwb_reg->mem_read_out >> reg_wr_src->get(RegWrSrc::MEMREAD);
        memwb_reg->alures_out >> reg_wr_src->get(RegWrSrc::ALURES);
        memwb_reg->pc4_out >> reg_wr_src->get(RegWrSrc::PC4);
        memwb_reg->reg_wr_src_ctrl_out >> reg_wr_src->select;

        registerFile->setMemory(m_regMem);

        // -----------------------------------------------------------------------
        // Branch
        idex_reg->br_op_out >> branch->comp_op;
        idex_reg->r1_out >> branch->op1;
        idex_reg->r2_out >> branch->op2;

        branch->res >> *br_and->in[0];
        idex_reg->do_br_out >> *br_and->in[1];
        br_and->out >> *controlflow_or->in[0];
        idex_reg->do_jmp_out >> *controlflow_or->in[1];

        pc_4->out >> pc_src->get(PcSrc::PC4);
        alu->res >> pc_src->get(PcSrc::ALU);

        // -----------------------------------------------------------------------
        // ALU
        idex_reg->r1_out >> alu_op1_src->get(AluSrc1::REG1);
        idex_reg->pc_out >> alu_op1_src->get(AluSrc1::PC);
        idex_reg->alu_op1_ctrl_out >> alu_op1_src->select;

        idex_reg->r2_out >> alu_op2_src->get(AluSrc2::REG2);
        idex_reg->imm_out >> alu_op2_src->get(AluSrc2::IMM);
        idex_reg->alu_op2_ctrl_out >> alu_op2_src->select;

        alu_op1_src->out >> alu->op1;
        alu_op2_src->out >> alu->op2;

        idex_reg->alu_ctrl_out >> alu->ctrl;

        // -----------------------------------------------------------------------
        // Data memory
        exmem_reg->alures_out >> data_mem->addr;
        exmem_reg->mem_do_write_out >> data_mem->wr_en;
        exmem_reg->r2_out >> data_mem->data_in;
        exmem_reg->mem_op_out >> data_mem->op;
        data_mem->mem->setMemory(m_memory);

        // -----------------------------------------------------------------------
        // Ecall checker
        decode->opcode >> ecallChecker->opcode;
        ecallChecker->setSysCallSignal(&handleSysCall);
        0 >> ecallChecker->stallEcallHandling;

        // -----------------------------------------------------------------------
        // IF/ID
        pc_4->out >> ifid_reg->pc4_in;
        pc_reg->out >> ifid_reg->pc_in;
        instr_mem->data_out >> ifid_reg->instr_in;
        1 >> ifid_reg->enable;
        efsc_or->out >> ifid_reg->clear;
        1 >> ifid_reg->valid_in;  // Always valid unless register is cleared

        // -----------------------------------------------------------------------
        // ID/EX
        1 >> idex_reg->enable;
        controlflow_or->out >> idex_reg->clear;

        // Data
        ifid_reg->pc4_out >> idex_reg->pc4_in;
        ifid_reg->pc_out >> idex_reg->pc_in;
        registerFile->r1_out >> idex_reg->r1_in;
        registerFile->r2_out >> idex_reg->r2_in;
        immediate->imm >> idex_reg->imm_in;

        // Control
        decode->wr_reg_idx >> idex_reg->wr_reg_idx_in;
        control->reg_wr_src_ctrl >> idex_reg->reg_wr_src_ctrl_in;
        control->reg_do_write_ctrl >> idex_reg->reg_do_write_in;
        control->alu_op1_ctrl >> idex_reg->alu_op1_ctrl_in;
        control->alu_op2_ctrl >> idex_reg->alu_op2_ctrl_in;
        control->mem_do_write_ctrl >> idex_reg->mem_do_write_in;
        control->mem_do_read_ctrl >> idex_reg->mem_do_read_in;
        control->alu_ctrl >> idex_reg->alu_ctrl_in;
        control->mem_ctrl >> idex_reg->mem_op_in;
        control->comp_ctrl >> idex_reg->br_op_in;
        control->do_branch >> idex_reg->do_br_in;
        control->do_jump >> idex_reg->do_jmp_in;

        ifid_reg->valid_out >> idex_reg->valid_in;

        // -----------------------------------------------------------------------
        // EX/MEM
        0 >> exmem_reg->clear;
        1 >> exmem_reg->enable;

        // Data
        idex_reg->pc_out >> exmem_reg->pc_in;
        idex_reg->pc4_out >> exmem_reg->pc4_in;
        idex_reg->r2_out >> exmem_reg->r2_in;
        alu->res >> exmem_reg->alures_in;

        // Control
        idex_reg->reg_wr_src_ctrl_out >> exmem_reg->reg_wr_src_ctrl_in;
        idex_reg->wr_reg_idx_out >> exmem_reg->wr_reg_idx_in;
        idex_reg->reg_do_write_out >> exmem_reg->reg_do_write_in;
        idex_reg->mem_do_write_out >> exmem_reg->mem_do_write_in;
        idex_reg->mem_op_out >> exmem_reg->mem_op_in;
        idex_reg->mem_do_read_out >> exmem_reg->mem_do_read_in;

        idex_reg->valid_out >> exmem_reg->valid_in;

        // -----------------------------------------------------------------------
        // MEM/WB

        // Data
        exmem_reg->pc_out >> memwb_reg->pc_in;
        exmem_reg->pc4_out >> memwb_reg->pc4_in;
        exmem_reg->alures_out >> memwb_reg->alures_in;
        data_mem->data_out >> memwb_reg->mem_read_in;

        // Control
        exmem_reg->reg_wr_src_ctrl_out >> memwb_reg->reg_wr_src_ctrl_in;
        exmem_reg->wr_reg_idx_out >> memwb_reg->wr_reg_idx_in;
        exmem_reg->reg_do_write_out >> memwb_reg->reg_do_write_in;

        exmem_reg->valid_out >> memwb_reg->valid_in;
    }

    // Design subcomponents
    SUBCOMPONENT(registerFile, RegisterFile<true>);
    SUBCOMPONENT(alu, ALU);
    SUBCOMPONENT(control, Control);
    SUBCOMPONENT(immediate, Immediate);
    SUBCOMPONENT(decode, Decode);
    SUBCOMPONENT(branch, Branch);
    SUBCOMPONENT(pc_4, Adder<RV_REG_WIDTH>);

    // Registers
    SUBCOMPONENT(pc_reg, Register<RV_REG_WIDTH>);

    // Stage seperating registers
    SUBCOMPONENT(ifid_reg, IFID);
    SUBCOMPONENT(idex_reg, IDEX);
    SUBCOMPONENT(exmem_reg, EXMEM);
    SUBCOMPONENT(memwb_reg, MEMWB);

    // Multiplexers
    SUBCOMPONENT(reg_wr_src, TYPE(EnumMultiplexer<RegWrSrc, RV_REG_WIDTH>));
    SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, RV_REG_WIDTH>));
    SUBCOMPONENT(alu_op1_src, TYPE(EnumMultiplexer<AluSrc1, RV_REG_WIDTH>));
    SUBCOMPONENT(alu_op2_src, TYPE(EnumMultiplexer<AluSrc2, RV_REG_WIDTH>));

    // Memories
    SUBCOMPONENT(instr_mem, TYPE(ROM<RV_REG_WIDTH, RV_INSTR_WIDTH>));
    SUBCOMPONENT(data_mem, TYPE(RVMemory<RV_REG_WIDTH, RV_REG_WIDTH>));

    // Gates
    // True if branch instruction and branch taken
    SUBCOMPONENT(br_and, TYPE(And<1, 2>));
    // True if branch taken or jump instruction
    SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));
    // True if controlflow action or performing syscall finishing
    SUBCOMPONENT(efsc_or, TYPE(Or<1, 2>));

    // Address spaces
    ADDRESSSPACE(m_memory);
    ADDRESSSPACE(m_regMem);

    SUBCOMPONENT(ecallChecker, EcallChecker);

    // Ripes interface compliance
    unsigned int stageCount() const override { return STAGECOUNT; }
    unsigned int getPcForStage(unsigned int idx) const override {
        // clang-format off
        switch (idx) {
            case IF: return pc_reg->out.uValue();
            case ID: return ifid_reg->pc_out.uValue();
            case EX: return idex_reg->pc_out.uValue();
            case MEM: return exmem_reg->pc_out.uValue();
            case WB: return memwb_reg->pc_out.uValue();
            default: assert(false && "Processor does not contain stage");
        }
        Q_UNREACHABLE();
        // clang-format on
    }
    unsigned int nextFetchedAddress() const override { return pc_src->out.uValue(); }
    QString stageName(unsigned int idx) const override {
        // clang-format off
        switch (idx) {
            case IF: return "IF";
            case ID: return "ID";
            case EX: return "EX";
            case MEM: return "MEM";
            case WB: return "WB";
            default: assert(false && "Processor does not contain stage");
        }
        Q_UNREACHABLE();
        // clang-format on
    }
    StageInfo stageInfo(unsigned int stage) const override {
        bool stageValid = true;
        // Has the pipeline stage been filled?
        stageValid &= stage <= m_cycleCount;

        // clang-format off
        // Has the stage been cleared?
        switch(stage){
        case ID: stageValid &= ifid_reg->valid_out.uValue(); break;
        case EX: stageValid &= idex_reg->valid_out.uValue(); break;
        case MEM: stageValid &= exmem_reg->valid_out.uValue(); break;
        case WB: stageValid &= memwb_reg->valid_out.uValue(); break;
        default: case IF: break;
        }

        // Is the stage carrying a valid (executable) PC?
        switch(stage){
        case ID: stageValid &= isExecutableAddress(ifid_reg->pc_out.uValue()); break;
        case EX: stageValid &= isExecutableAddress(idex_reg->pc_out.uValue()); break;
        case MEM: stageValid &= isExecutableAddress(exmem_reg->pc_out.uValue()); break;
        case WB: stageValid &= isExecutableAddress(memwb_reg->pc_out.uValue()); break;
        default: case IF: stageValid &= isExecutableAddress(pc_reg->out.uValue()); break;
        }

        // Are we currently clearing the pipeline due to a syscall exit? if such, all stages before the EX stage are invalid
        if(stage < EX){
            stageValid &= !ecallChecker->isSysCallExiting();
        }
        // clang-format on

        // Gather stage state info
        StageInfo::State state = StageInfo ::State::None;
        switch (stage) {
            case IF:
                break;
            case ID:
                if (m_cycleCount > ID && ifid_reg->valid_out.uValue() == 0) {
                    state = StageInfo::State::Flushed;
                }
                break;
            case EX: {
                if (m_cycleCount > EX && idex_reg->valid_out.uValue() == 0) {
                    state = StageInfo::State::Flushed;
                }
                break;
            }
            case MEM: {
                if (m_cycleCount > MEM && exmem_reg->valid_out.uValue() == 0) {
                    state = StageInfo::State::Flushed;
                }
                break;
            }
            case WB: {
                if (m_cycleCount > WB && memwb_reg->valid_out.uValue() == 0) {
                    state = StageInfo::State::Flushed;
                }
                break;
            }
        }

        return StageInfo({getPcForStage(stage), stageValid, state});
    }

    void setProgramCounter(uint32_t address) override {
        pc_reg->forceValue(0, address);
        propagateDesign();
    }
    void setPCInitialValue(uint32_t address) override { pc_reg->setInitValue(address); }
    SparseArray& getMemory() override { return *m_memory; }
    unsigned int getRegister(RegisterFileType rfid, unsigned i) const override { return registerFile->getRegister(i); }
    SparseArray& getArchRegisters() override { return *m_regMem; }
    void finalize(const FinalizeReason& fr) override {
        if (fr.exitSyscall && !ecallChecker->isSysCallExiting()) {
            // An exit system call was executed. Record the cycle of the execution, and enable the ecallChecker's system
            // call exiting signal.
            m_syscallExitCycle = m_cycleCount;
        }
        ecallChecker->setSysCallExiting(ecallChecker->isSysCallExiting() || fr.exitSyscall);
    }

    const Component* getDataMemory() const override { return data_mem; }
    const Component* getInstrMemory() const override { return instr_mem; }

    bool finished() const override {
        // The processor is finished when there are no more valid instructions in the pipeline
        bool allStagesInvalid = true;
        for (int stage = IF; stage < STAGECOUNT; stage++) {
            allStagesInvalid &= !stageInfo(stage).stage_valid;
            if (!allStagesInvalid)
                break;
        }
        return allStagesInvalid;
    }
    void setRegister(RegisterFileType rfid, unsigned i, uint32_t v) override {
        setSynchronousValue(registerFile->_wr_mem, i, v);
    }

    void clock() override {
        // An instruction has been retired if the instruction in the WB stage is valid and the PC is within the
        // executable range of the program
        if (memwb_reg->valid_out.uValue() != 0 && isExecutableAddress(memwb_reg->pc_out.uValue())) {
            m_instructionsRetired++;
        }

        RipesProcessor::clock();
    }

    void reverse() override {
        if (m_syscallExitCycle != -1 && (m_cycleCount - 1) == m_syscallExitCycle) {
            // We are about to undo an exit syscall instruction. In this case, the syscall exiting sequence should be
            // terminate
            ecallChecker->setSysCallExiting(false);
            m_syscallExitCycle = -1;
        }
        RipesProcessor::reverse();
        if (memwb_reg->valid_out.uValue() != 0 && isExecutableAddress(memwb_reg->pc_out.uValue())) {
            m_instructionsRetired--;
        }
    }

    void reset() override {
        RipesProcessor::reset();
        ecallChecker->setSysCallExiting(false);
        m_syscallExitCycle = -1;
    }

    static const ISAInfoBase* ISA() {
        static auto s_isa = ISAInfo<ISA::RV32I>(QStringList{"M"});
        return &s_isa;
    }

    const ISAInfoBase* supportsISA() const override { return ISA(); };
    const ISAInfoBase* implementsISA() const override { return m_enabledISA.get(); };

    const std::set<RegisterFileType> registerFiles() const override {
        std::set<RegisterFileType> rfs;
        rfs.insert(RegisterFileType::GPR);

        if (implementsISA()->extensionEnabled("F")) {
            rfs.insert(RegisterFileType::FPR);
        }
        return rfs;
    }

private:
    /**
     * @brief m_syscallExitCycle
     * The variable will contain the cycle of which an exit system call was executed. From this, we may determine when
     * we roll back an exit system call during rewinding.
     */
    long long m_syscallExitCycle = -1;
    std::shared_ptr<ISAInfo<ISA::RV32I>> m_enabledISA;
};

}  // namespace core
}  // namespace vsrtl
