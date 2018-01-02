#include "pipeline.h"

Pipeline::Pipeline() {
    registerRegs();

    // Connect pipeline

    // ------ IF ---------
    // Connect alu to PC and a constant "4" signal, and set operator to "Add"
    c_4 = Signal<32>(4);
    m_add_pc4.setInputs(r_PC_IF.getOutput(), &c_4);
    r_instr_IFID.setInput(&instr_IF);

    // ------ ID ---------
    m_reg.setInputs(r_instr_IFID.getOutput(), r_writeReg_MEMWB.getOutput(), mux_memToReg.getOutput());
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

void Pipeline::step() {
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

    // IF - Opted not to create an "Instruction memory" object
    m_add_pc4.update();
    instr_IF = Signal<32>(m_memory.read((uint32_t)r_PC_IF));  // Read instruction at current PC
    writeReg = Signal<5>(((uint32_t)r_instr_IFID >> 7) & 0b11111);

    // Clock all registers
    clock();
}

void Pipeline::clock() {
    // Clocks all registered registers
    for (const auto& reg : m_regs) {
        reg->clock();
    }
}
void Pipeline::reset() {
    m_memory.clear();
}

void Pipeline::restart() {}
