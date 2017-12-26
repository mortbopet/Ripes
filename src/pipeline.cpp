#include "pipeline.h"

Pipeline::Pipeline() {
    registerRegs();

    // Build pipeline

    // ------ IF ---------
    // Connect alu to PC and a constant "4" signal, and set operator to "Add"
    c_4 = Signal<32>(4);
    alu_pc4.setInputs(std::vector<Signal<32>*>({r_PC.getOutput(), &c_4}));
}

void Pipeline::registerRegs() {
    // Registers all registers in m_regs
    m_regs.push_back(&r_IFID);
}

void Pipeline::immGen() {
    // Generates an immediate value on the basis of an instruction opcode
    // Opcode bits 5 and 6 can define the required fields for generating the immediate
    switch (((uint32_t)r_IFID & 0b1100000) >> 5) {
        case 0b00: {
            // Load instruction, sign extend bits 31:20
            imm_ID = Signal<32>(signextend<int32_t, 12>(((uint32_t)r_IFID >> 20)));
            break;
        }
        case 0b01: {
            // Store instruction
            uint32_t v(r_IFID);
            imm_ID = Signal<32>(signextend<int32_t, 12>(((v & 0xfe000000)) >> 20) | ((v & 0xf80) >> 7));
            break;
        }
        default: {
            // Conditional branch instructions
            uint32_t v(r_IFID);
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

    // IF
    alu_pc4.update();
    instr_IF = Signal<32>(m_memory.read((uint32_t)r_PC));

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
