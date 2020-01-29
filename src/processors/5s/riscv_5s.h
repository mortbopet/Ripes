#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "../ripesprocessor.h"

// Functional units
#include "../ss/alu.h"
#include "../ss/branch.h"
#include "../ss/control.h"
#include "../ss/decode.h"
#include "../ss/ecallchecker.h"
#include "../ss/immediate.h"
#include "../ss/registerfile.h"
#include "../ss/riscv.h"
#include "../ss/rvmemory.h"

// Stage separating registers
#include "5s_exmem.h"
#include "5s_idex.h"
#include "5s_ifid.h"
#include "5s_memwb.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

class FiveStageRISCV : public RipesProcessor {
public:
    FiveStageRISCV() : RipesProcessor("5-Stage RISC-V Processor") {
        // -----------------------------------------------------------------------
        // Program counter
        pc_reg->out >> pc_4->op1;
        4 >> pc_4->op2;
        pc_src->out >> pc_reg->in;

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
        control->comp_ctrl >> branch->comp_op;
        idex_reg->r1_out >> branch->op1;
        idex_reg->r2_out >> branch->op2;

        branch->res >> *br_and->in[0];
        control->do_branch >> *br_and->in[1];
        br_and->out >> *controlflow_or->in[0];
        control->do_jump >> *controlflow_or->in[1];

        pc_4->out >> pc_src->get(PcSrc::PC4);
        jmpTarget->out >> pc_src->get(PcSrc::ALU);

        idex_reg->pc4_out >> jmpTarget->op1;
        idex_reg->imm_out >> jmpTarget->op2;

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

        // -----------------------------------------------------------------------
        // IF/ID
        pc_4->out >> ifid_reg->pc4_in;
        pc_reg->out >> ifid_reg->pc_in;
        instr_mem->data_out >> ifid_reg->instr_in;
        1 >> ifid_reg->enable;
        0 >> ifid_reg->clear;

        // -----------------------------------------------------------------------
        // ID/EX
        1 >> idex_reg->enable;
        0 >> idex_reg->clear;

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
        control->alu_ctrl >> idex_reg->alu_ctrl_in;
        control->mem_ctrl >> idex_reg->mem_op_in;

        // -----------------------------------------------------------------------
        // EX/MEM

        // Data
        idex_reg->pc_out >> exmem_reg->pc_in;
        idex_reg->pc4_out >> exmem_reg->pc4_in;
        jmpTarget->out >> exmem_reg->jmpTarget_in;
        idex_reg->r2_out >> exmem_reg->r2_in;
        alu->res >> exmem_reg->alures_in;

        // Control
        idex_reg->reg_wr_src_ctrl_out >> exmem_reg->reg_wr_src_ctrl_in;
        idex_reg->wr_reg_idx_out >> exmem_reg->wr_reg_idx_in;
        idex_reg->reg_do_write_out >> exmem_reg->reg_do_write_in;
        idex_reg->mem_do_write_out >> exmem_reg->mem_do_write_in;
        idex_reg->mem_op_out >> exmem_reg->mem_op_in;

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
    }

    // Design subcomponents
    SUBCOMPONENT(registerFile, RegisterFile);
    SUBCOMPONENT(alu, ALU);
    SUBCOMPONENT(control, Control);
    SUBCOMPONENT(immediate, Immediate);
    SUBCOMPONENT(decode, Decode);
    SUBCOMPONENT(branch, Branch);
    SUBCOMPONENT(pc_4, Adder<RV_REG_WIDTH>);
    SUBCOMPONENT(jmpTarget, Adder<RV_REG_WIDTH>);

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
    SUBCOMPONENT(br_and, TYPE(And<1, 2>));
    SUBCOMPONENT(controlflow_or, TYPE(Or<1, 2>));

    // Address spaces
    ADDRESSSPACE(m_memory);
    ADDRESSSPACE(m_regMem);

    SUBCOMPONENT(ecallChecker, EcallChecker);

    // Ripes interface compliance
    virtual const ISAInfoBase* implementsISA() const override { return ISAInfo<ISA::RV32IM>::instance(); }
    unsigned int stageCount() const override { return 5; }
    unsigned int getPcForStage(unsigned int idx) const override {
        switch (idx) {
            case 0:
                return pc_reg->out.uValue();
            case 1:
                return ifid_reg->pc_out.uValue();
            case 2:
                return idex_reg->pc_out.uValue();
            case 3:
                return exmem_reg->pc_out.uValue();
            case 4:
                return memwb_reg->pc_out.uValue();
            default:
                return 0;
        }
    }
    unsigned int nextFetchedAddress() const override { return pc_src->out.uValue(); }
    QString stageName(unsigned int idx) const override {
        switch (idx) {
            case 0:
                return "IF";
            case 1:
                return "ID";
            case 2:
                return "EX";
            case 3:
                return "MEM";
            case 4:
                return "WB";
            default:
                return "N/A";
        }
    }
    StageInfo stageInfo(unsigned int idx) const override { return StageInfo({getPcForStage(idx), true}); }
    void setProgramCounter(uint32_t address) override {
        pc_reg->forceValue(0, address);
        propagateDesign();
    }
    void setPCInitialValue(uint32_t address) override { pc_reg->setInitValue(address); }
    SparseArray& getMemory() override { return *m_memory; }
    unsigned int getRegister(unsigned i) const override { return registerFile->getRegister(i); }
    SparseArray& getRegisters() override { return *m_regMem; }
    void finalize() override {
        // Allow one additional clock cycle to clear the current instruction
        m_finishInNextCycle = true;
    }
    bool finished() const override { return m_finished; }

    void setRegister(unsigned i, uint32_t v) override { setSynchronousValue(registerFile->_wr_mem, i, v); }

    void clock() override {
        // Single cycle processor; 1 instruction retired per cycle!
        m_instructionsRetired++;

        // m_finishInNextCycle may be set during Design::clock(). Store the value before clocking the processor, and
        // emit finished if this was the final clock cycle.
        const bool finishInThisCycle = m_finishInNextCycle;
        RipesProcessor::clock();
        if (finishInThisCycle) {
            m_finished = true;
        }
    }

    void reverse() override {
        m_instructionsRetired--;
        RipesProcessor::reverse();
        // Ensure that reverses performed when we expected to finish in the following cycle, clears this
        // expectation.
        m_finishInNextCycle = false;
        m_finished = false;
    }

    void reset() override {
        RipesProcessor::reset();
        m_finishInNextCycle = false;
        m_finished = false;
    }

private:
    bool m_finishInNextCycle = false;
    bool m_finished = false;
};

}  // namespace core
}  // namespace vsrtl
