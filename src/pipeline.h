#ifndef PIPELINE_H
#define PIPELINE_H

#include "mainmemory.h"
#include "pipelineobjects.h"

class Pipeline {
public:
    Pipeline();
    // Utility functions
    void restart();
    void reset();
    void step();

    static Pipeline* getPipeline() {
        static Pipeline pipeline;
        return &pipeline;
    }

private:
    // Simulator functions
    void immGen();
    void aluCtrl();
    void clock();

    // Memory
    Registers m_reg;
    MainMemory m_memory;

    // Combinatorial items & signals
    ALU<32> m_add_pc4;  // ALU for incrementing PC by 4
    Signal<32> c_4 = Signal<32>("4");

    ALU<32> m_add_target;  // ALU for PC target computation

    ALU<32> m_alu;  // Arithmetic ALU
    Signal<CTRL_SIZE> alu_ctrl;

    Mux<2, 32> mux_memToReg;
    Signal<1> ctrl_memToReg;

    Signal<32> imm_ID = Signal<32>("Immediate");
    Signal<32> instr_IF = Signal<32>("Instruction");
    Signal<32> readData_MEM;

    // Registers
    std::vector<RegBase*> m_regs;
    Reg<32> r_PC_IF, r_instr_IFID, r_rd1_IDEX, r_rd2_IDEX, r_imm_IDEX, r_alures_EXMEM, r_writeData_EXMEM,
        r_readData_MEMWB, r_alures_MEMWB;
    // Write register value is passed from instruction through all stages
    Signal<5> writeReg;
    Reg<5> r_writeReg_IDEX, r_writeReg_EXMEM, r_writeReg_MEMWB;
};

#endif  // PIPELINE_H
