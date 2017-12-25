#ifndef PIPELINE_H
#define PIPELINE_H

#include "pipelineobjects.h"

class Pipeline {
public:
    Pipeline();

private:
    void immGen();

    // Signals
    Signal<64> imm;

    // Registers
    Reg<32> r_IFID;
};

#endif  // PIPELINE_H
