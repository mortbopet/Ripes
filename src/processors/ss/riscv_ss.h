#pragma once

#include "VSRTL/core/vsrtl_adder.h"
#include "VSRTL/core/vsrtl_constant.h"
#include "VSRTL/core/vsrtl_design.h"
#include "VSRTL/core/vsrtl_logicgate.h"
#include "VSRTL/core/vsrtl_multiplexer.h"

#include "ripesprocessor.h"

#include "alu.h"
#include "branch.h"
#include "control.h"
#include "decode.h"
#include "immediate.h"
#include "registerfile.h"
#include "riscv.h"
#include "rvmemory.h"

namespace vsrtl {
using namespace core;

namespace RISCV {

class SingleCycleRISCV : public Ripes::RipesProcessor {
public:
    SingleCycleRISCV() : Ripes::RipesProcessor("Single Cycle RISC-V Processor") {
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

        // -----------------------------------------------------------------------
        // Decode
        instr_mem->data_out >> decode->instr;

        // -----------------------------------------------------------------------
        // Control signals
        decode->opcode >> control->opcode;

        // -----------------------------------------------------------------------
        // Immediate
        decode->opcode >> immediate->opcode;
        instr_mem->data_out >> immediate->instr;

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
    }

    // Design subcomponents
    SUBCOMPONENT(registerFile, RegisterFile);
    SUBCOMPONENT(alu, ALU);
    SUBCOMPONENT(control, Control);
    SUBCOMPONENT(immediate, Immediate);
    SUBCOMPONENT(decode, Decode);
    SUBCOMPONENT(branch, Branch);
    SUBCOMPONENT(pc_4, Adder<RV_REG_WIDTH>);

    // Registers
    SUBCOMPONENT(pc_reg, Register<RV_REG_WIDTH>);

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
};
}  // namespace RISCV
}  // namespace vsrtl
