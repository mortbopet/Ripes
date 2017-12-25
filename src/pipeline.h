#ifndef PIPELINE_H
#define PIPELINE_H

#include "pipelineobjects.h"

class Pipeline {
public:
    Pipeline();

private:
    void registerRegs();
    void immGen();
    void clock();

    // Signals
    Signal<64> imm;

    // Registers
    std::vector<RegBase*> m_regs;
    Reg<32> r_IFID;
};

#endif  // PIPELINE_H
