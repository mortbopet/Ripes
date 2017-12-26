#ifndef PIPELINE_H
#define PIPELINE_H

#include "mainmemory.h"
#include "pipelineobjects.h"

class Pipeline {
public:
    Pipeline();

private:
    // Utility functions
    void restart();
    void reset();

    // Simulator functions
    void registerRegs();
    void immGen();
    void clock();
    void step();

    // Objects
    MainMemory m_memory;

    ALU<32> alu_pc4;

    // Signals - Stage name is appended to name
    Signal<32> c_4;
    Signal<32> imm_ID;
    Signal<32> instr_IF;
    // Registers
    std::vector<RegBase*> m_regs;
    Reg<32> r_IFID;
    Reg<32> r_PC;
};

#endif  // PIPELINE_H
