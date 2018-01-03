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
    int getTextSize() const { return m_textSize; }
    bool isReady() const { return m_ready; }

    static Pipeline* getPipeline() {
        static Pipeline pipeline;
        return &pipeline;
    }

private:
    // Simulator functions
    void propagateCombinational();
    void immGen();
    void controlGen();
    void aluCtrl();
    void clock();
    bool m_ready = false;

    // Program counters for each stage
    void setStagePCS();
    StagePCS m_pcs;
    bool m_finished = false;

    // Memory
    Registers m_reg;
    MainMemory m_memory;
    uint32_t m_textSize = 0;

    // Combinatorial items & signals
    ALU<32> alu_pc4 = ALU<32>("ADD PC 4");  // ALU for incrementing PC by 4
    Signal<32> c_4 = Signal<32>("4");

    ALU<32> alu_pc_target = ALU<32>("ALU PC target");  // ALU for PC target computation

    ALU<32> alu_mainALU = ALU<32>("Main ALU");  // Arithmetic ALU
    Signal<ALUDefs::CTRL_SIZE> alu_ctrl;
    Mux<2, 32> mux_ALUSrc;
    Mux<2, 32> mux_memToReg;
    Signal<1> ctrl_memToReg;

    Signal<32> imm_ID = Signal<32>("Immediate");
    Signal<32> instr_IF = Signal<32>("Instruction");
    Signal<32> readData_MEM;

    // Registers
    std::vector<RegBase*> m_regs;
    Reg<32> r_instr_IFID, r_rd1_IDEX, r_rd2_IDEX, r_imm_IDEX, r_alures_EXMEM, r_writeData_EXMEM, r_readData_MEMWB,
        r_alures_MEMWB;
    Reg<32> r_PC_IF, r_PC_IFID, r_PC_IDEX, r_PC_EXMEM, r_PC_MEMWB;
    Signal<5> writeReg;
    Reg<5> r_writeReg_IDEX, r_writeReg_EXMEM, r_writeReg_MEMWB;  // Write register (# of register to write to)

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
    Signal<1> s_RegWrite, s_ALUSrc, s_MemToReg;
    Signal<ALUDefs::CTRL_SIZE> s_ALUOP;

    // Control signal enums
    enum MemRead { LB = 1, LH = 2, LW = 3, LBU = 4, LHU = 5 };
    enum MemWrite { SB = 1, SH = 2, SW = 3 };
};

#endif  // PIPELINE_H
