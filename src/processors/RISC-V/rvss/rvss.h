#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "../../ripesvsrtlprocessor.h"

#include "../riscv.h"
#include "../rv_alu.h"
#include "../rv_branch.h"
#include "../rv_control.h"
#include "../rv_ecallchecker.h"
#include "../rv_immediate.h"
#include "../rv_memory.h"
#include "../rv_registerfile.h"
#include "rv_decodeRVC.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

template <typename XLEN_T>
class RVSS : public RipesVSRTLProcessor {
    static_assert(std::is_same<uint32_t, XLEN_T>::value || std::is_same<uint64_t, XLEN_T>::value,
                  "Only supports 32- and 64-bit variants");
    static constexpr unsigned XLEN = sizeof(XLEN_T) * CHAR_BIT;

public:
    RVSS(const QStringList& extensions) : RipesVSRTLProcessor("Single Cycle RISC-V Processor") {
        m_enabledISA = std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(extensions);
        decode->setISA(m_enabledISA);

        // -----------------------------------------------------------------------
        // Program counter
        pc_reg->out >> pc_4->op1;
        pc_inc->out >> pc_4->op2;
        pc_src->out >> pc_reg->in;

        2 >> pc_inc->get(PcInc::INC2);
        4 >> pc_inc->get(PcInc::INC4);
        decode->Pc_Inc >> pc_inc->select;

        // Note: pc_src works uses the PcSrc enum, but is selected by the boolean signal
        // from the controlflow OR gate. PcSrc enum values must adhere to the boolean
        // 0/1 values.
        controlflow_or->out >> pc_src->select;

        // -----------------------------------------------------------------------
        // Instruction memory
        pc_reg->out >> instr_mem->addr;
        instr_mem->setMemory(m_memory);

        // -----------------------------------------------------------------------
        // Decode
        instr_mem->data_out >> decode->instr;

        // -----------------------------------------------------------------------
        // Control signals
        decode->opcode >> control->opcode;

        // -----------------------------------------------------------------------
        // Immediate
        decode->opcode >> immediate->opcode;
        decode->exp_instr >> immediate->instr;

        // -----------------------------------------------------------------------
        // Registers
        decode->wr_reg_idx >> registerFile->wr_addr;
        decode->r1_reg_idx >> registerFile->r1_addr;
        decode->r2_reg_idx >> registerFile->r2_addr;
        control->reg_do_write_ctrl >> registerFile->wr_en;
        reg_wr_src->out >> registerFile->data_in;

        data_mem->data_out >> reg_wr_src->get(RegWrSrc::MEMREAD);
        alu->res >> reg_wr_src->get(RegWrSrc::ALURES);
        pc_4->out >> reg_wr_src->get(RegWrSrc::PC4);
        control->reg_wr_src_ctrl >> reg_wr_src->select;

        registerFile->setMemory(m_regMem);

        // -----------------------------------------------------------------------
        // Branch
        control->comp_ctrl >> branch->comp_op;
        registerFile->r1_out >> branch->op1;
        registerFile->r2_out >> branch->op2;

        branch->res >> *br_and->in[0];
        control->do_branch >> *br_and->in[1];
        br_and->out >> *controlflow_or->in[0];
        control->do_jump >> *controlflow_or->in[1];
        pc_4->out >> pc_src->get(PcSrc::PC4);
        alu->res >> pc_src->get(PcSrc::ALU);

        // -----------------------------------------------------------------------
        // ALU
        registerFile->r1_out >> alu_op1_src->get(AluSrc1::REG1);
        pc_reg->out >> alu_op1_src->get(AluSrc1::PC);
        control->alu_op1_ctrl >> alu_op1_src->select;

        registerFile->r2_out >> alu_op2_src->get(AluSrc2::REG2);
        immediate->imm >> alu_op2_src->get(AluSrc2::IMM);
        control->alu_op2_ctrl >> alu_op2_src->select;

        alu_op1_src->out >> alu->op1;
        alu_op2_src->out >> alu->op2;

        control->alu_ctrl >> alu->ctrl;

        // -----------------------------------------------------------------------
        // Data memory
        alu->res >> data_mem->addr;
        control->mem_do_write_ctrl >> data_mem->wr_en;
        registerFile->r2_out >> data_mem->data_in;
        control->mem_ctrl >> data_mem->op;
        data_mem->mem->setMemory(m_memory);

        // -----------------------------------------------------------------------
        // Ecall checker
        decode->opcode >> ecallChecker->opcode;
        ecallChecker->setSyscallCallback(&trapHandler);
        0 >> ecallChecker->stallEcallHandling;
    }

    // Design subcomponents
    SUBCOMPONENT(registerFile, TYPE(RegisterFile<XLEN, false>));
    SUBCOMPONENT(alu, TYPE(ALU<XLEN>));
    SUBCOMPONENT(control, Control);
    SUBCOMPONENT(immediate, TYPE(Immediate<XLEN>));
    SUBCOMPONENT(decode, TYPE(DecodeRVC<XLEN>));
    SUBCOMPONENT(branch, TYPE(Branch<XLEN>));
    SUBCOMPONENT(pc_4, Adder<XLEN>);

    // Registers
    SUBCOMPONENT(pc_reg, Register<XLEN>);

    // Multiplexers
    SUBCOMPONENT(reg_wr_src, TYPE(EnumMultiplexer<RegWrSrc, XLEN>));
    SUBCOMPONENT(pc_src, TYPE(EnumMultiplexer<PcSrc, XLEN>));
    SUBCOMPONENT(alu_op1_src, TYPE(EnumMultiplexer<AluSrc1, XLEN>));
    SUBCOMPONENT(alu_op2_src, TYPE(EnumMultiplexer<AluSrc2, XLEN>));
    SUBCOMPONENT(pc_inc, TYPE(EnumMultiplexer<PcInc, XLEN>));

    // Memories
    SUBCOMPONENT(instr_mem, TYPE(ROM<XLEN, c_RVInstrWidth>));
    SUBCOMPONENT(data_mem, TYPE(RVMemory<XLEN, XLEN>));

    // Gates
    SUBCOMPONENT(br_and, TYPE(And<1, 2>));
    SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));

    // Address spaces
    ADDRESSSPACEMM(m_memory);
    ADDRESSSPACE(m_regMem);

    SUBCOMPONENT(ecallChecker, EcallChecker);

    // Ripes interface compliance
    unsigned int stageCount() const override { return 1; }
    unsigned int getPcForStage(unsigned int) const override { return pc_reg->out.uValue(); }
    AInt nextFetchedAddress() const override { return pc_src->out.uValue(); }
    QString stageName(unsigned int) const override { return "•"; }
    StageInfo stageInfo(unsigned int) const override {
        return StageInfo({pc_reg->out.uValue(), isExecutableAddress(pc_reg->out.uValue()), StageInfo::State::None});
    }
    void setProgramCounter(AInt address) override {
        pc_reg->forceValue(0, address);
        propagateDesign();
    }
    void setPCInitialValue(AInt address) override { pc_reg->setInitValue(address); }
    AddressSpaceMM& getMemory() override { return *m_memory; }
    VInt getRegister(RegisterFileType, unsigned i) const override { return registerFile->getRegister(i); }
    void finalize(FinalizeReason fr) override {
        if (fr == FinalizeReason::exitSyscall) {
            // Allow one additional clock cycle to clear the current instruction
            m_finishInNextCycle = true;
        }
    }
    bool finished() const override { return m_finished || !stageInfo(0).stage_valid; }
    const std::vector<unsigned> breakpointTriggeringStages() const override { return {0}; }

    MemoryAccess dataMemAccess() const override { return memToAccessInfo(data_mem); }
    MemoryAccess instrMemAccess() const override {
        auto instrAccess = memToAccessInfo(instr_mem);
        instrAccess.type = MemoryAccess::Read;
        return instrAccess;
    }

    void setRegister(RegisterFileType, unsigned i, VInt v) override {
        setSynchronousValue(registerFile->_wr_mem, i, v);
    }

    void clockProcessor() override {
        // Single cycle processor; 1 instruction retired per cycle!
        m_instructionsRetired++;

        // m_finishInNextCycle may be set during Design::clock(). Store the value before clocking the processor, and
        // emit finished if this was the final clock cycle.
        const bool finishInThisCycle = m_finishInNextCycle;
        Design::clock();
        if (finishInThisCycle) {
            m_finished = true;
        }
    }

    void reverse() override {
        m_instructionsRetired--;
        Design::reverse();
        // Ensure that reverses performed when we expected to finish in the following cycle, clears this expectation.
        m_finishInNextCycle = false;
        m_finished = false;
    }

    void reset() override {
        Design::reset();
        m_finishInNextCycle = false;
        m_finished = false;
    }

    static ProcessorISAInfo supportsISA() {
        return ProcessorISAInfo{std::make_shared<ISAInfo<XLenToRVISA<XLEN>()>>(QStringList()), {"M", "C"}, {"M"}};
    }
    const ISAInfoBase* implementsISA() const override { return m_enabledISA.get(); }
    const std::set<RegisterFileType> registerFiles() const override {
        std::set<RegisterFileType> rfs;
        rfs.insert(RegisterFileType::GPR);

        // @TODO: uncomment when enabling floating-point support
        // if (implementsISA()->extensionEnabled("F")) {
        //     rfs.insert(RegisterFileType::Float);
        // }
        return rfs;
    }

private:
    bool m_finishInNextCycle = false;
    bool m_finished = false;
    std::shared_ptr<ISAInfoBase> m_enabledISA;
};

}  // namespace core
}  // namespace vsrtl
