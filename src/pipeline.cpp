#include "pipeline.h"

Pipeline::Pipeline() {
    registerRegs();
}

void Pipeline::registerRegs() {
    // Registers all registers in m_regs
    m_regs.push_back(&r_IFID);
}

void Pipeline::immGen() {
    // Generates an immediate value on the basis of an instruction opcode
    // Opcode bits 5 and 6 can define the required fields for generating the immediate
    switch ((r_IFID.value() & 0b1100000) >> 5) {
        case 0b00: {
            // Load instruction, sign extend bits 31:20
            imm = Signal<64>(signextend<int32_t, 12>((r_IFID.value() >> 20)));
            break;
        }
        case 0b01: {
            // Store instruction
            uint32_t v = r_IFID.value();
            imm = Signal<64>(signextend<int32_t, 12>(((v & 0xfe000000)) >> 20) | ((v & 0xf80) >> 7));
            break;
        }
        default: {
            // Conditional branch instructions
            uint32_t v = r_IFID.value();
            imm = Signal<64>(signextend<int32_t, 13>(((v & 0x80000000)) >> 20) | ((v & 0x7e) >> 20) |
                             ((v & 0xf00) >> 7) | ((v & 0x80) << 6));
            break;
        }
    }
}

void Pipeline::clock() {
    // Clocks all registered registers
    for (const auto& reg : m_regs) {
        reg->clock();
    }
}
