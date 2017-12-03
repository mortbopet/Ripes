#ifndef RUNNER_H
#define RUNNER_H

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "defines.h"
#include "runnercache.h"

#include "parser.h"

class Runner {
public:
    runnerState exec();
    runnerState step();
    static Runner* getRunner() {
        static Runner runner(Parser::getParser());
        return &runner;
    }

    RunnerCache* getRunnerCachePtr() { return &m_cache; }
    memory* getMemoryPtr() { return &m_memory; }
    std::vector<uint32_t>* getRegPtr() { return &m_reg; }
    int getTextSize() const { return m_textSize; }
    const StagePCS& getStagePCS() const { return m_stagePCS; }
    bool isReady() { return m_ready; }

    void reset();
    void restart();
    void update();

private:
    Runner(Parser* parser);
    ~Runner();
    runnerState execInstruction(Instruction instr);
    void setStageInstructions();
    void handleError(runnerState err) const;

    int m_pc = 0;  // program counter

    // Memory - Memory is interfaced through a single function, but allocated
    // seperately. A symbolic pointer is set up to create the virtual index into
    // the total memory
    std::vector<uint32_t> m_reg;  // Internal registers
    const uint32_t m_textStart = 0x0;
    std::vector<uint8_t> m_data;
    const uint32_t m_dataStart = 0x10000000;
    std::vector<uint8_t> m_stack;
    const uint32_t m_stackStart = 0x7ffffff0;
    std::vector<uint8_t> m_heap;
    const uint32_t m_heapStart = 0x10008000;

    memory m_memory;

    void memWrite(uint32_t address, uint32_t value, int size);
    uint32_t memRead(uint32_t address);

    uint32_t m_textSize = 0;

    Parser* m_parser;
    bool m_running = false;  // flag for disabling UI update signals when running simulator
    bool m_ready;

    bool getInstruction(int pc);
    Instruction m_currentInstruction;
    StagePCS m_stagePCS;  // container for which instruction is in each pipeline stage

    // Instruction execution functions
    runnerState execLuiInstr(Instruction instr);
    runnerState execAuipcInstr(Instruction instr);
    runnerState execJalInstr(Instruction instr);
    runnerState execJalrInstr(Instruction instr);
    runnerState execBranchInstr(Instruction instr);
    runnerState execLoadInstr(Instruction instr);
    runnerState execStoreInstr(Instruction instr);
    runnerState execOpImmInstr(Instruction instr);
    runnerState execOpInstr(Instruction instr);
    runnerState execEcallInstr();

    // Cache
    RunnerCache m_cache;
};

#endif  // RUNNER_H
