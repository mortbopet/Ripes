#include "pipeline.h"

Pipeline::Pipeline() {
    // Connect pipeline

    // ------ IF ---------
    // Connect alu to PC and a constant "4" signal, and set operator to "Add"
    c_4 = Signal<32>(4);
    m_add_pc4.setInputs(r_PC_IF.getOutput(), &c_4);
    m_add_pc4.setControl(&alu_const_add);
    r_PC_IF.setInput(m_add_pc4.getOutput());
    r_instr_IFID.setInput(&instr_IF);

    // ------ ID ---------
    m_reg.setInputs(r_instr_IFID.getOutput(), r_writeReg_MEMWB.getOutput(), mux_memToReg.getOutput(), &m_regWrite);
    r_imm_IDEX.setInput(&imm_ID);
    r_rd1_IDEX.setInput(m_reg.getOutput(1));
    r_rd2_IDEX.setInput(m_reg.getOutput(2));
    r_writeReg_IDEX.setInput(&writeReg);

    // ------ EX ---------
    m_alu.setInputs(r_rd1_IDEX.getOutput(), r_rd2_IDEX.getOutput());
    m_alu.setControl(&alu_ctrl);
    r_alures_EXMEM.setInput(m_alu.getOutput());
    r_writeData_EXMEM.setInput(r_rd2_IDEX.getOutput());
    r_writeReg_EXMEM.setInput(r_writeReg_IDEX.getOutput());

    // ------ MEM --------
    r_readData_MEMWB.setInput(&readData_MEM);
    r_alures_MEMWB.setInput(r_alures_EXMEM.getOutput());
    r_writeReg_MEMWB.setInput(r_writeReg_EXMEM.getOutput());

    // ------ WB ---------
    mux_memToReg.setInput(0, r_alures_MEMWB.getOutput());
    mux_memToReg.setInput(1, r_readData_MEMWB.getOutput());
    mux_memToReg.setControl(&ctrl_memToReg);

    // Program counters
    r_PC_IFID.setInput(r_PC_IF.getOutput());
    r_PC_IDEX.setInput(r_PC_IFID.getOutput());
    r_PC_EXMEM.setInput(r_PC_IDEX.getOutput());
    r_PC_MEMWB.setInput(r_PC_EXMEM.getOutput());
}

void Pipeline::immGen() {
    // Generates an immediate value on the basis of an instruction opcode
    // Opcode bits 5 and 6 can define the required fields for generating the immediate
    switch (((uint32_t)r_instr_IFID & 0b1100000) >> 5) {
        case 0b00: {
            // Load instruction, sign extend bits 31:20
            imm_ID = Signal<32>(signextend<int32_t, 12>(((uint32_t)r_instr_IFID >> 20)));
            break;
        }
        case 0b01: {
            // Store instruction
            uint32_t v(r_instr_IFID);
            imm_ID = Signal<32>(signextend<int32_t, 12>(((v & 0xfe000000)) >> 20) | ((v & 0xf80) >> 7));
            break;
        }
        default: {
            // Conditional branch instructions
            uint32_t v(r_instr_IFID);
            imm_ID = Signal<32>(signextend<int32_t, 13>(((v & 0x80000000)) >> 20) | ((v & 0x7e) >> 20) |
                                ((v & 0xf00) >> 7) | ((v & 0x80) << 6));
            break;
        }
    }
}

int Pipeline::step() {
    // Main processing loop for the pipeline
    // propagates the signals throgh the combinational logic and clocks the
    // sequential logic afterwards

    // The order of component updating must be done in the correct sequential order!
    // The rightmost stage is updated first, going from left to right in the given stage.
    // This is to ensure that feedback signals are valid

    // WB
    mux_memToReg.update();

    // MEM

    // EX
    m_alu.update();

    // ID
    immGen();
    m_reg.update();
    writeReg = Signal<5>(((uint32_t)r_instr_IFID >> 7) & 0b11111);

    // IF - Opted not to create an "Instruction memory" object
    m_add_pc4.update();
    // Load nops if PC is greater than text size
    instr_IF = Signal<32>(
        (uint32_t)r_PC_IF > m_textSize ? 0 : m_memory.read((uint32_t)r_PC_IF));  // Read instruction at current PC

    // Set stage program counters
    setStagePCS();

    // Execution is finished if nops are in all stages except WB
    if (m_pcs.WB.first == m_textSize) {
        return 1;
    }

    // Clock all registers
    RegBase::clockAll();
    return 0;
}

#define PCVAL(pc) std::pair<uint32_t, bool>((uint32_t)pc, true)
void Pipeline::setStagePCS() {
    // To validate a PC value (whether there is actually an instruction in the stage, or if the pipeline has been
    // reset), the previous stage PC is used to determine the current state of a stage
    // To facilitate this, the PCS are set in reverse order
    m_pcs.WB = m_pcs.MEM.second ? PCVAL(r_PC_MEMWB) : m_pcs.WB;
    m_pcs.MEM = m_pcs.EX.second ? PCVAL(r_PC_EXMEM) : m_pcs.MEM;
    m_pcs.EX = m_pcs.ID.second ? PCVAL(r_PC_IDEX) : m_pcs.EX;
    m_pcs.ID = m_pcs.IF.second ? PCVAL(r_PC_IFID) : m_pcs.ID;
    m_pcs.IF = PCVAL(r_PC_IF);
}

int Pipeline::run() {
    while (!step())
        ;
    return 1;
}

void Pipeline::reset() {
    // Called when resetting the simulator (loading a new program)
    restart();
    m_memory.clear();
    m_textSize = 0;
    m_ready = false;
}

void Pipeline::update() {
    // Must be called whenever external changes to the memory has been envoked
    m_textSize = m_memory.size();
    m_ready = true;
    restart();
}

void Pipeline::restart() {
    // Called when restarting a simulation
    m_reg.clear();
    m_memory.reset(m_textSize);
    m_reg.init();
    m_pcs.reset();
    RegBase::resetAll();
}
