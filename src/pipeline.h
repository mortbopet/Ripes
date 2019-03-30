#ifndef PIPELINE_H
#define PIPELINE_H

#include "mainmemory.h"
#include "pipelineobjects.h"

#include <set>

class Pipeline {
    friend class PipelineWidget;
    friend class RWJumpModel;

public:
    // Control signal enums
    enum CompOp { BEQ = 1, BNE = 2, BLT = 3, BLTU = 4, BGE = 5, BGEU = 6 };
    enum MemRead { LB = 1, LH = 2, LW = 3, LBU = 4, LHU = 5 };
    enum MemWrite { SB = 1, SH = 2, SW = 3 };
    enum ECALL { none, print_int = 1, print_char = 2, print_string = 4, exit = 10 };
    enum PCSRC { PC4, BR, JALR };
    Pipeline();
    // Utility functions
    void restart();
    void reset();
    void update();
    int step();
    int run();
    void abort();
    void clearAbort();
    bool isRunning() const { return m_running; }

    // Pointers for GUI
    std::set<uint32_t>* getBreakpoints() { return &m_breakpoints; }
    MainMemory* getRuntimeMemoryPtr() { return &m_runtimeMemory; }
    void setBaselineMemory(const MainMemory& memory) { m_baselineMemory = memory; }
    std::unordered_map<uint32_t, uint8_t>* getDataMemoryPtr() { return &m_dataMemory; }
    std::vector<uint32_t>* getRegPtr() { return m_reg.getRegPtr(); }
    std::pair<Pipeline::ECALL, int32_t> checkEcall(bool reset = true);
    const StagePCS& getStagePCS() const { return m_pcs; }
    const StagePCS& getStagePCSPre() const { return m_pcsPre; }
    int getTextSize() const { return m_textSize; }
    bool isReady() const { return m_ready; }
    bool isFinished() const { return m_finished; }
    const std::vector<StagePCS>& getPcsList() { return m_pcsCycles; }
    int getCycleCount() const { return m_cycleCount; }
    int getInstructionsExecuted() const { return m_instructionsExecuted; }

    static Pipeline* getPipeline() {
        static Pipeline pipeline;
        return &pipeline;
    }

    std::set<uint32_t> m_breakpoints;

private:
    // Simulator functions
    void propagateCombinational();
    void immGen();
    void controlGen();
    void forwardingControlGen();
    void handleEcall();
    void hazardControlGen();
    void aluCtrl();
    void clock();
    void doFinishCleanup();
    bool m_ready = false;
    bool m_running;
    ECALL m_ecallArg;
    int32_t m_ecallVal;

    unsigned int m_instructionsExecuted = 0;
    unsigned int m_cycleCount = 0;

    // Program counters for each stage
    void setStagePCS();
    StagePCS m_pcs;
    StagePCS m_pcsPre;
    bool m_finishing =
        false;  // Set when ecall 10 is detected, and the pipeline is running its remaining instructions through
    bool m_finished = false;
    bool m_abort = false;
    int m_finishingCnt = 0;
    std::vector<StagePCS> m_pcsCycles;       // list of PCs for each cycle
    std::vector<RVAccess> m_MemoryAccesses;  // List of all read/write accesses to memory

    /** Memory
     * Runtime memory: Memory as used by the processor during execution. Also used for GUI visualization of the memory
     * state Baseline memory: Memory as it is in the initial state of loading the program Having separate runtime and
     * baseline memory allows us to easily reset the simulator upon resetting the simulation, in case programs alter the
     * program (.text segment) during execution
     */
    Registers m_reg;
    MainMemory m_runtimeMemory;
    MainMemory m_baselineMemory;

    std::unordered_map<uint32_t, uint8_t> m_dataMemory;  // Since the data memory needs to be restarted on a simulator
                                                         // reset, the assembler-provided data segment must be stored
    uint32_t m_textSize = 0;

    // Combinatorial items & signals
    ALU<32> alu_pc4 = ALU<32>("ADD PC 4");  // ALU for incrementing PC by 4
    Signal<32> c_4 = Signal<32>(4, "4");
    Mux<3, 32> mux_PCSrc;
    Signal<1> s_branchTaken;
    ALU<32> alu_pc_target = ALU<32>("ALU PC target");  // ALU for PC target computation

    ALU<32> alu_mainALU = ALU<32>("Main ALU");  // Arithmetic ALU
    Mux<2, 32> mux_ALUSrc1;
    Mux<2, 32> mux_ALUSrc2;
    Mux<3, 32> mux_memToReg;
    Signal<32> s_imm_ID = Signal<32>("Immediate");
    Signal<32> s_instr_IF = Signal<32>("Instruction");
    Signal<32> readData_MEM;

    Signal<5> s_readRegister1, s_readRegister2;

    Mux<3, 32> mux_forwardA_EX, mux_forwardB_EX, mux_forwardA_ID,
        mux_forwardB_ID;  // Forwarding mux'es for execute stage and ID stage (branch comparison operation)

    Signal<2> s_invalidPC;

    Mux<2, 32> mux_alures_PC4_MEM;

    // Registers
    Reg<5> r_readRegister1_IDEX, r_readRegister2_IDEX;
    Reg<32> r_instr_IFID, r_rd1_IDEX, r_rd2_IDEX, r_imm_IDEX, r_alures_EXMEM, r_writeData_EXMEM, r_readData_MEMWB,
        r_alures_MEMWB;
    Reg<32> r_PC_IF, r_PC_IFID, r_PC_IDEX, r_PC_EXMEM, r_PC_MEMWB;
    Reg<32> r_PC4_IFID, r_PC4_IDEX, r_PC4_EXMEM, r_PC4_MEMWB;
    Reg<2> r_invalidPC_IFID, r_invalidPC_IDEX, r_invalidPC_EXMEM, r_invalidPC_MEMWB;
    Reg<1> r_jal_IDEX, r_jal_EXMEM, r_jalr_IDEX, r_jalr_EXMEM;

    Signal<5> writeReg;
    Reg<5> r_writeReg_IDEX, r_writeReg_EXMEM, r_writeReg_MEMWB;  // Write register (# of register to write to)
    RegBank bank_IDEX;

    // Control propagation registers
    Reg<1> r_regWrite_IDEX, r_regWrite_EXMEM, r_regWrite_MEMWB;  // Register write signal
    Reg<ALUOps::CTRL_SIZE> r_ALUOP_IDEX;
    Reg<1> r_ALUSrc1_IDEX, r_ALUSrc2_IDEX;   // Multiplexor control signals
    Reg<3> r_MemRead_IDEX, r_MemRead_EXMEM;  // Write/read from memory signals. Funct3 value for instruction determines
                                             // signal value (ie. byte width and signed/unsigned load)
    Reg<2> r_MemWrite_IDEX, r_MemWrite_EXMEM, r_memToReg_IDEX, r_memToReg_EXMEM, r_memToReg_MEMWB;

    // Control signals
    Signal<ALUOps::CTRL_SIZE> s_alu_const_add = Signal<ALUOps::CTRL_SIZE>(
        (uint32_t)ALUOps::OPCODE::ADD);  // A constant "ADD" control signal, to turn an ALU into an adder
    Signal<3> s_MemRead;
    Signal<2> s_MemWrite;
    Signal<2> s_memToReg;
    Signal<1> s_RegWrite;
    Signal<1> s_ALUSrc1, s_ALUSrc2;  // Multiplexor control signal for ALU input (2:1)
    Signal<2> s_forwardA_EX, s_forwardB_EX, s_forwardA_ID,
        s_forwardB_ID;  // Multiplexor control signal for ALU and ID register forwarding (3:1)
    Signal<1> s_Branch;
    Signal<ALUOps::CTRL_SIZE> s_ALUOP;
    Signal<3> s_CompOp;
    Signal<1> s_validPC;  // used for GUI to determine whether a PC = 0 is a nop (register flush) or valid
    Signal<1> s_PCWrite;  // Used when a branch instruction requires a pipeline stall
    Signal<1> s_IFID_write, s_IFID_reset;  // Used when clearing IFID after branching or jumping
    Signal<1> s_IDEX_reset;                // Used when a branch instruction requires a pipeline stall
    Signal<2> s_PCSrc;
    Signal<1> s_jal, s_jalr;
    Signal<1> s_auipc;           // used for controlling input 1 to main alu (registers or PC)
    Signal<1> s_alures_PC4_MEM;  // Control signal for forwarding MUX in MEM stage
};

namespace Forwarding {
enum MuxForward { NONE = 0, EXMEM = 1, MEMWB = 2 };
}
namespace MemToReg {
enum values { MEMREAD = 0, ALURES = 1, PC4 = 2 };
}

namespace HazardReason {
enum values { NONE = 0, BRANCHTAKEN = 1, STALL = 2, eof = 3 };
}

#endif  // PIPELINE_H
