#include "pipeline.h"

#include "parser.h"

Pipeline::Pipeline() {
    // Connect pipeline

    // ------ IF ---------
    // Connect alu to PC and a constant "4" signal, and set operator to "Add"
    c_4 = Signal<32>(4);
    alu_pc4.setInputs(r_PC_IF.getOutput(), &c_4);
    alu_pc4.setControl(&s_alu_const_add);
    r_PC_IF.setInput(alu_pc4.getOutput());
    r_instr_IFID.setInput(&instr_IF);

    // ------ ID ---------
    m_reg.setInputs(r_instr_IFID.getOutput(), r_writeReg_MEMWB.getOutput(), mux_memToReg.getOutput(),
                    r_regWrite_MEMWB.getOutput());
    r_imm_IDEX.setInput(&imm_ID);
    r_rd1_IDEX.setInput(m_reg.getOutput(1));
    r_rd2_IDEX.setInput(m_reg.getOutput(2));
    r_writeReg_IDEX.setInput(&writeReg);

    // Control signals
    r_regWrite_IDEX.setInput(&s_RegWrite);
    r_ALUOP_IDEX.setInput(&s_ALUOP);
    r_ALUSrc_IDEX.setInput(&s_ALUSrc);
    r_MemWrite_IDEX.setInput(&s_MemWrite);
    r_MemRead_IDEX.setInput(&s_MemRead);
    r_MemtoReg_IDEX.setInput(&s_MemtoReg);

    // ------ EX ---------
    alu_mainALU.setInputs(r_rd1_IDEX.getOutput(), mux_ALUSrc.getOutput());
    alu_mainALU.setControl(&alu_ctrl);
    mux_ALUSrc.setInput(0, r_rd2_IDEX.getOutput());
    mux_ALUSrc.setInput(1, r_imm_IDEX.getOutput());
    mux_ALUSrc.setControl(r_ALUSrc_IDEX.getOutput());
    r_alures_EXMEM.setInput(alu_mainALU.getOutput());
    r_writeData_EXMEM.setInput(r_rd2_IDEX.getOutput());
    r_writeReg_EXMEM.setInput(r_writeReg_IDEX.getOutput());

    // Control signals
    r_regWrite_EXMEM.setInput(r_regWrite_IDEX.getOutput());
    r_MemWrite_EXMEM.setInput(r_MemWrite_IDEX.getOutput());
    r_MemRead_EXMEM.setInput(r_MemRead_IDEX.getOutput());
    r_MemtoReg_EXMEM.setInput(r_MemtoReg_IDEX.getOutput());

    // ------ MEM --------
    r_readData_MEMWB.setInput(&readData_MEM);
    r_alures_MEMWB.setInput(r_alures_EXMEM.getOutput());
    r_writeReg_MEMWB.setInput(r_writeReg_EXMEM.getOutput());

    // Control signals
    r_regWrite_MEMWB.setInput(r_regWrite_EXMEM.getOutput());
    r_MemtoReg_MEMWB.setInput(r_MemtoReg_EXMEM.getOutput());

    // ------ WB ---------
    mux_memToReg.setInput(0, r_alures_MEMWB.getOutput());
    mux_memToReg.setInput(1, r_readData_MEMWB.getOutput());
    mux_memToReg.setControl(r_MemtoReg_MEMWB.getOutput());

    // Program counters
    r_PC_IFID.setInput(r_PC_IF.getOutput());
    r_PC_IDEX.setInput(r_PC_IFID.getOutput());
    r_PC_EXMEM.setInput(r_PC_IDEX.getOutput());
    r_PC_MEMWB.setInput(r_PC_EXMEM.getOutput());
}

void Pipeline::immGen() {
    if ((uint32_t)r_instr_IFID & 0b1111111 == 0b0110111) {
        // LUI
        imm_ID = Signal<32>((uint32_t)r_instr_IFID & 0xfffff000);
    } else {
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
}

namespace {
#define CTRL_I_TYPE \
    s_ALUSrc = 1;   \
    s_MemtoReg = 0; \
    s_RegWrite = 1; \
    s_MemRead = 0;  \
    s_MemWrite = 0;

#define CTRL_R_TYPE \
    s_ALUSrc = 0;   \
    s_MemtoReg = 0; \
    s_RegWrite = 1; \
    s_MemRead = 0;  \
    s_MemWrite = 0;
}

void Pipeline::controlGen() {
    // Generates control signals for the pipeline based on the input instruction
    switch ((uint32_t)r_instr_IFID & 0b1111111) {
        case 0b0110111: {
            // LUI
            s_ALUOP = ALUDefs::LUI;
            CTRL_I_TYPE
            break;
        }
        case 0b0010011: {
            // I-Type
            auto fields = Parser::getParser()->decodeIInstr((uint32_t)r_instr_IFID);
            switch (fields[2]) {
                case 0b000: {
                    // ADDI
                    s_ALUOP = ALUDefs::ADD;
                    CTRL_I_TYPE
                    break;
                }
                case 0b010: {
                    // SLTI
                    s_ALUOP = ALUDefs::LT;
                    CTRL_I_TYPE
                    break;
                }
                case 0b011: {
                    // SLTIU
                    s_ALUOP = ALUDefs::LTU;
                    CTRL_I_TYPE
                    break;
                }
                case 0b100: {
                    // XORI
                    s_ALUOP = ALUDefs::XOR;
                    CTRL_I_TYPE
                    break;
                }
                case 0b110: {
                    // ORI
                    s_ALUOP = ALUDefs::OR;
                    CTRL_I_TYPE
                    break;
                }
                case 0b111: {
                    // ANDI
                    s_ALUOP = ALUDefs::AND;
                    CTRL_I_TYPE
                    break;
                }
            }
            break;
        }
        case 0b0110011: {
            // R-type
            auto fields = Parser::getParser()->decodeRInstr((uint32_t)r_instr_IFID);
            switch (fields[3]) {
                case 0b000: {
                    switch (fields[0]) {
                        case 0b0000000: {
                            // ADD
                            s_ALUOP = ALUDefs::ADD;
                            CTRL_R_TYPE
                            break;
                        }
                        case 0100000: {
                            // SUB
                            s_ALUOP = ALUDefs::SUB;
                            CTRL_R_TYPE
                            break;
                        }
                    }
                    break;
                }
                case 0b001: {
                    // SLL
                    s_ALUOP = ALUDefs::SL;
                    CTRL_R_TYPE
                    break;
                }
                case 0b010: {
                    // SLT
                    s_ALUOP = ALUDefs::LT;
                    CTRL_R_TYPE
                    break;
                }
                case 0b011: {
                    // SLTU
                    s_ALUOP = ALUDefs::LTU;
                    CTRL_R_TYPE
                    break;
                }
                case 0b100: {
                    // XOR
                    s_ALUOP = ALUDefs::XOR;
                    CTRL_R_TYPE
                    break;
                }
                case 0b101: {
                    switch (fields[0]) {
                        case 0b0000000: {
                            // SRL
                            s_ALUOP = ALUDefs::SRL;
                            CTRL_R_TYPE
                            break;
                        }
                        case 0b0100000: {
                            // SRA
                            s_ALUOP = ALUDefs::SRA;
                            CTRL_R_TYPE
                            break;
                        }
                    }
                    break;
                }
                case 0b110: {
                    // OR
                    s_ALUOP = ALUDefs::OR;
                    CTRL_R_TYPE
                    break;
                }
                case 0b111: {
                    // AND
                    s_ALUOP = ALUDefs::AND;
                    CTRL_R_TYPE
                    break;
                }
            }
            break;
        }
        default: {
            s_ALUOP = 0;
            s_ALUSrc = 0;
            s_MemtoReg = 0;
            s_RegWrite = 0;
            s_MemRead = 0;
            s_MemWrite = 0;
            break;
        }
    }
}

void Pipeline::propagateCombinational() {
    // The order of component updating must be done in the correct sequential order!
    // The rightmost stage is updated first, going from left to right in the given stage.
    // This is to ensure that feedback signals are valid
    // ----- WB -----
    mux_memToReg.update();

    // ----- MEM -----
    if (r_MemWrite_EXMEM) {
    } else if (r_MemRead_EXMEM) {
    }

    // ----- EX -----
    mux_ALUSrc.update();
    alu_mainALU.update();

    // ----- ID -----
    controlGen();
    immGen();
    m_reg.update();
    writeReg = Signal<5>(((uint32_t)r_instr_IFID >> 7) & 0b11111);

    // ----- IF -----
    alu_pc4.update();
    // Load nops if PC is greater than text size
    instr_IF = Signal<32>(
        (uint32_t)r_PC_IF > m_textSize ? 0 : m_memory.read((uint32_t)r_PC_IF));  // Read instruction at current PC
}

int Pipeline::step() {
    // Main processing loop for the pipeline
    // propagates the signals throgh the combinational logic and clocks the
    // sequential logic afterwards

    // Clock all registers
    m_reg.clock();  // write register values. Must happen before clocking all registers (to apply the correct regWrite
                    // value)
    RegBase::clockAll();

    // Propagate signals throuh logic
    propagateCombinational();

    // Set stage program counters
    setStagePCS();

    // Execution is finished if nops are in all stages except WB
    if (m_pcs.WB.first == m_textSize) {
        return 1;
    } else {
        return 0;
    }
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

    // Reset all registers to 0 and propagate signals through combinational logic
    RegBase::resetAll();
    propagateCombinational();

    // set PC_IF stage to the first instruction
    m_pcs.IF = PCVAL(r_PC_IF);
}
