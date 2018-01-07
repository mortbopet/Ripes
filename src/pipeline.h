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
    void update();
    int step();
    int run();

    // Pointers for GUI
    MainMemory* getMemoryPtr() { return &m_memory; }
    std::vector<uint32_t>* getRegPtr() { return m_reg.getRegPtr(); }
    const StagePCS& getStagePCS() const { return m_pcs; }
    const StagePCS& getStagePCSPre() const { return m_pcsPre; }
    int getTextSize() const { return m_textSize; }
    bool isReady() const { return m_ready; }
    bool isFinished() const { return m_finished; }

    static Pipeline* getPipeline() {
        static Pipeline pipeline;
        return &pipeline;
    }

private:
    // Simulator functions
    void propagateCombinational();
    void immGen();
    void controlGen();
    void forwardingControlGen();
    void hazardControlGen();
    void aluCtrl();
    void clock();
    bool m_ready = false;

    // Program counters for each stage
    void setStagePCS();
    StagePCS m_pcs;
    StagePCS m_pcsPre;
    bool m_finished = false;

    // Memory
    Registers m_reg;
    MainMemory m_memory;
    uint32_t m_textSize = 0;

    // Combinatorial items & signals
    ALU<32> alu_pc4 = ALU<32>("ADD PC 4");  // ALU for incrementing PC by 4
    Signal<32> c_4 = Signal<32>(4, "4");
    Mux<2, 32> mux_PCSrc;
    Signal<1> s_PCSrc;
    ALU<32> alu_pc_target = ALU<32>("ALU PC target");  // ALU for PC target computation

    ALU<32> alu_mainALU = ALU<32>("Main ALU");  // Arithmetic ALU
    Mux<2, 32> mux_ALUSrc;
    Mux<2, 32> mux_memToReg;
    Signal<1> ctrl_memToReg;

    Signal<32> s_imm_ID = Signal<32>("Immediate");
    Signal<32> s_instr_IF = Signal<32>("Instruction");
    Signal<32> readData_MEM;

    Signal<5> s_readRegister1, s_readRegister2;

    Mux<3, 32> mux_forwardA_EX, mux_forwardB_EX, mux_forwardA_ID,
        mux_forwardB_ID;  // Forwarding mux'es for execute stage and ID stage (branch comparison operation)
    Signal<2> s_forwardA_EX, s_forwardB_EX, s_forwardA_ID, s_forwardB_ID;
    Signal<1> s_invalidPC;

    // Registers
    Reg<5> r_readRegister1_IDEX, r_readRegister2_IDEX;
    std::vector<RegBase*> m_regs;
    Reg<32> r_instr_IFID, r_rd1_IDEX, r_rd2_IDEX, r_imm_IDEX, r_alures_EXMEM, r_writeData_EXMEM, r_readData_MEMWB,
        r_alures_MEMWB;
    Reg<32> r_PC_IF, r_PC_IFID, r_PC_IDEX, r_PC_EXMEM, r_PC_MEMWB;
    Reg<1> r_invalidPC_IFID, r_invalidPC_IDEX, r_invalidPC_EXMEM, r_invalidPC_MEMWB;
    Signal<5> writeReg;
    Reg<5> r_writeReg_IDEX, r_writeReg_EXMEM, r_writeReg_MEMWB;  // Write register (# of register to write to)
    RegBank bank_IDEX;

    // Control propagation registers
    Reg<1> r_regWrite_IDEX, r_regWrite_EXMEM, r_regWrite_MEMWB;  // Register write signal
    Reg<ALUDefs::CTRL_SIZE> r_ALUOP_IDEX;
    Reg<1> r_ALUSrc_IDEX, r_MemtoReg_IDEX, r_MemtoReg_EXMEM, r_MemtoReg_MEMWB;  // Multiplexor control signals
    Reg<3> r_MemRead_IDEX, r_MemRead_EXMEM;  // Write/read from memory signals. Funct3 value for instruction determines
                                             // signal value (ie. byte width and signed/unsigned load)
    Reg<2> r_MemWrite_IDEX, r_MemWrite_EXMEM;

    // Control signals
    Signal<ALUDefs::CTRL_SIZE> s_alu_const_add = Signal<ALUDefs::CTRL_SIZE>(
        (uint32_t)ALUDefs::OPCODE::ADD);  // A constant "ADD" control signal, to turn an ALU into an adder
    Signal<3> s_MemRead;
    Signal<2> s_MemWrite;
    Signal<1> s_RegWrite, s_ALUSrc, s_MemToReg, s_Branch;
    Signal<ALUDefs::CTRL_SIZE> s_ALUOP;
    Signal<3> s_CompOp;
    Signal<1> s_validPC;     // used for GUI to determine whether a PC = 0 is a nop (register flush) or valid
    Signal<1> s_PCWrite;     // Used when a branch instruction requires a pipeline stall
    Signal<1> s_IFID_write;  // Used when a branch instruction requires a pipeline stall
    Signal<1> s_IDEX_reset;  // Used when a branch instruction requires a pipeline stall

    // Control signal enums
    enum CompOp { BEQ = 1, BNE = 2, BLT = 3, BLTU = 4, BGE = 5, BGEU = 6 };
    enum MemRead { LB = 1, LH = 2, LW = 3, LBU = 4, LHU = 5 };
    enum MemWrite { SB = 1, SH = 2, SW = 3 };
};
namespace Forwarding {
enum MuxForward { NONE = 0, EXMEM = 1, MEMWB = 2 };
}

#endif  // PIPELINE_H
