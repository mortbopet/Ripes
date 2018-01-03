#include "pipeline.h"

#include "parser.h"

#include <QDebug>

Pipeline::Pipeline() {
    // Connect pipeline

    // ------ IF ---------
    // Connect alu to PC and a constant "4" signal, and set operator to "Add"
    alu_pc4.setInputs(r_PC_IF.getOutput(), &c_4);
    alu_pc4.setControl(&s_alu_const_add);
    r_PC_IF.setInput(mux_PCSrc.getOutput());
    r_instr_IFID.setInput(&s_instr_IF);
    r_instr_IFID.setReset(&s_PCSrc);
    mux_PCSrc.setControl(&s_PCSrc);
    mux_PCSrc.setInput(0, alu_pc4.getOutput());
    mux_PCSrc.setInput(1, alu_pc_target.getOutput());

    // ------ ID ---------
    m_reg.setInputs(r_instr_IFID.getOutput(), r_writeReg_MEMWB.getOutput(), mux_memToReg.getOutput(),
                    r_regWrite_MEMWB.getOutput());
    r_imm_IDEX.setInput(&imm_ID);
    r_rd1_IDEX.setInput(m_reg.getOutput(1));
    r_rd2_IDEX.setInput(m_reg.getOutput(2));
    r_writeReg_IDEX.setInput(&writeReg);

    alu_pc_target.setControl(&s_alu_const_add);
    alu_pc_target.setInputs(r_PC_IDEX.getOutput(), &imm_ID_shifted);

    // Control signals
    r_regWrite_IDEX.setInput(&s_RegWrite);
    r_ALUOP_IDEX.setInput(&s_ALUOP);
    r_ALUSrc_IDEX.setInput(&s_ALUSrc);
    r_MemWrite_IDEX.setInput(&s_MemWrite);
    r_MemRead_IDEX.setInput(&s_MemRead);
    r_MemtoReg_IDEX.setInput(&s_MemToReg);

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
    if (((uint32_t)r_instr_IFID & 0b1111111) == 0b0110111) {
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
    imm_ID_shifted = imm_ID;  //((uint32_t)imm_ID) << 1;
}

namespace {
#define CTRL_I_TYPE \
    s_ALUSrc = 1;   \
    s_MemToReg = 0; \
    s_RegWrite = 1; \
    s_MemRead = 0;  \
    s_MemWrite = 0; \
    s_Branch = 0;   \
    s_CompOp = 0;

#define CTRL_R_TYPE \
    s_ALUSrc = 0;   \
    s_MemToReg = 0; \
    s_RegWrite = 1; \
    s_MemRead = 0;  \
    s_MemWrite = 0; \
    s_Branch = 0;   \
    s_CompOp = 0;

#define CTRL_STORE  \
    s_ALUSrc = 1;   \
    s_MemToReg = 0; \
    s_RegWrite = 0; \
    s_MemRead = 0;  \
    s_Branch = 0;   \
    s_CompOp = 0;

#define CTRL_LOAD   \
    s_ALUSrc = 1;   \
    s_MemToReg = 1; \
    s_RegWrite = 1; \
    s_MemWrite = 0; \
    s_Branch = 0;   \
    s_CompOp = 0;

#define CTRL_BRANCH \
    s_ALUSrc = 0;   \
    s_MemToReg = 0; \
    s_RegWrite = 0; \
    s_MemWrite = 0; \
    s_Branch = 1;
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
                    break;
                }
                case 0b010: {
                    // SLTI
                    s_ALUOP = ALUDefs::LT;
                    break;
                }
                case 0b011: {
                    // SLTIU
                    s_ALUOP = ALUDefs::LTU;
                    break;
                }
                case 0b100: {
                    // XORI
                    s_ALUOP = ALUDefs::XOR;
                    break;
                }
                case 0b110: {
                    // ORI
                    s_ALUOP = ALUDefs::OR;
                    break;
                }
                case 0b111: {
                    // ANDI
                    s_ALUOP = ALUDefs::AND;
                    break;
                }
            }
            CTRL_I_TYPE
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
                            break;
                        }
                        case 0100000: {
                            // SUB
                            s_ALUOP = ALUDefs::SUB;
                            break;
                        }
                    }
                    break;
                }
                case 0b001: {
                    // SLL
                    s_ALUOP = ALUDefs::SL;
                    break;
                }
                case 0b010: {
                    // SLT
                    s_ALUOP = ALUDefs::LT;
                    break;
                }
                case 0b011: {
                    // SLTU
                    s_ALUOP = ALUDefs::LTU;
                    break;
                }
                case 0b100: {
                    // XOR
                    s_ALUOP = ALUDefs::XOR;
                    break;
                }
                case 0b101: {
                    switch (fields[0]) {
                        case 0b0000000: {
                            // SRL
                            s_ALUOP = ALUDefs::SRL;
                            break;
                        }
                        case 0b0100000: {
                            // SRA
                            s_ALUOP = ALUDefs::SRA;
                            break;
                        }
                    }
                    break;
                }
                case 0b110: {
                    // OR
                    s_ALUOP = ALUDefs::OR;
                    break;
                }
                case 0b111: {
                    // AND
                    s_ALUOP = ALUDefs::AND;
                    break;
                }
            }
            CTRL_R_TYPE
            break;
        }
        case 0b0000011: {
            // Load instructions
            auto fields = Parser::getParser()->decodeIInstr((uint32_t)r_instr_IFID);
            switch (fields[2]) {
                case 0b000: {
                    // LB
                    s_MemRead = LB;
                    break;
                }
                case 0b001: {
                    // LH
                    s_MemRead = LH;
                    break;
                }
                case 0b010: {
                    // LW
                    s_MemRead = LW;
                    break;
                }
                case 0b100: {
                    // LBU
                    s_MemRead = LBU;
                    break;
                }
                case 0b101: {
                    // LHU
                    s_MemRead = LHU;
                    break;
                }
            }
            CTRL_LOAD
            break;
        }
        case 0b0100011: {
            // Store instructions
            auto fields = Parser::getParser()->decodeSInstr((uint32_t)r_instr_IFID);
            switch (fields[3]) {
                case 0b000: {
                    // SB
                    s_MemWrite = SB;
                    break;
                }
                case 0b001: {
                    // SH
                    s_MemWrite = SH;
                    break;
                }
                case 0b010: {
                    // SW
                    s_MemWrite = SW;
                    break;
                }
            }
            CTRL_STORE
            break;
        }
        case 0b1100011: {
            // Branch instruction
            auto fields = Parser::getParser()->decodeBInstr((uint32_t)r_instr_IFID);
            switch (fields[4]) {
                case 0b000: {
                    // BEQ
                    s_CompOp = CompOp::BEQ;
                    break;
                }
                case 0b001: {
                    // BNE
                    s_CompOp = CompOp::BNE;
                    break;
                }
                case 0b100: {
                    // BLT
                    s_CompOp = CompOp::BLT;
                    break;
                }
                case 0b101: {
                    // BGE
                    s_CompOp = CompOp::BGE;
                    break;
                }
                case 0b110: {
                    // BLTU
                    s_CompOp = CompOp::BLTU;
                    break;
                }
                case 0b111: {
                    // BGEU
                    s_CompOp = CompOp::BGEU;
                    break;
                }
            }
            CTRL_BRANCH
            break;
        }
        default: {
            s_ALUOP = 0;
            s_ALUSrc = 0;
            s_MemToReg = 0;
            s_RegWrite = 0;
            s_MemRead = 0;
            s_MemWrite = 0;
            s_Branch = 0;
            s_CompOp = 0;
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
    if (r_MemRead_EXMEM) {
        switch ((uint32_t)r_MemRead_EXMEM) {
            case LB: {
                readData_MEM = m_memory.read((uint32_t)r_alures_EXMEM) & 0xff;
                break;
            }
            case LH: {
                readData_MEM = m_memory.read((uint32_t)r_alures_EXMEM) & 0xffff;
                break;
            }
            case LW: {
                readData_MEM = m_memory.read((uint32_t)r_alures_EXMEM);
                break;
            }
            case LBU: {
                readData_MEM = signextend<int32_t, 8>(m_memory.read((uint32_t)r_alures_EXMEM) & 0xff);
                break;
            }
            case LHU: {
                readData_MEM = signextend<int32_t, 16>(m_memory.read((uint32_t)r_alures_EXMEM) & 0xffff);
                break;
            }
        }
    }

    // ----- EX -----
    mux_ALUSrc.update();
    alu_mainALU.update();

    // ----- ID -----
    controlGen();
    immGen();
    alu_pc_target.update();
    m_reg.update();
    writeReg = Signal<5>(((uint32_t)r_instr_IFID >> 7) & 0b11111);

    // Compare read register values and '&' with s_branch control signal
    switch ((CompOp)(int)s_CompOp) {
        case BEQ: {
            s_PCSrc = s_Branch && ((uint32_t)*m_reg.getOutput(1) == (uint32_t)*m_reg.getOutput(2));
            break;
        }
        case BNE: {
            s_PCSrc = s_Branch && ((uint32_t)*m_reg.getOutput(1) != (uint32_t)*m_reg.getOutput(2));
            break;
        }
        case BLT: {
            s_PCSrc = s_Branch && ((int32_t)*m_reg.getOutput(1) < (int32_t)*m_reg.getOutput(2));
            break;
        }
        case BLTU: {
            s_PCSrc = s_Branch && ((uint32_t)*m_reg.getOutput(1) < (uint32_t)*m_reg.getOutput(2));
            break;
        }
        case BGE: {
            s_PCSrc = s_Branch && ((int32_t)*m_reg.getOutput(1) >= (int32_t)*m_reg.getOutput(2));
            break;
        }
        case BGEU: {
            s_PCSrc = s_Branch && ((uint32_t)*m_reg.getOutput(1) >= (uint32_t)*m_reg.getOutput(2));
            break;
        }
        default: { s_PCSrc = 0; }
    }

    // ----- IF -----
    alu_pc4.update();
    mux_PCSrc.update();
    if ((uint32_t)s_PCSrc == 1) {
        qDebug() << "a";
    }
    // Load nops if PC is greater than text size
    s_instr_IF = Signal<32>(
        (uint32_t)r_PC_IF > m_textSize ? 0 : m_memory.read((uint32_t)r_PC_IF));  // Read instruction at current PC
}

int Pipeline::step() {
    // Main processing function for the pipeline
    // propagates the signals throgh the combinational logic and clocks the
    // sequential logic afterwards - similar to clocking a circuit

    // Clock inter-stage registers
    m_reg.clock();
    if (r_MemWrite_EXMEM) {
        switch ((uint32_t)r_MemWrite_EXMEM) {
            case SB: {
                m_memory.write((uint32_t)r_alures_EXMEM, (uint32_t)r_writeData_EXMEM, 1);
                break;
            }
            case SH: {
                m_memory.write((uint32_t)r_alures_EXMEM, (uint32_t)r_writeData_EXMEM, 2);
                break;
            }
            case SW: {
                m_memory.write((uint32_t)r_alures_EXMEM, (uint32_t)r_writeData_EXMEM, 4);
                break;
            }
        }
    }

    // Clock stage-separating registers
    RegBase::clockAll();

    // Propagate signals through logic
    propagateCombinational();

    // Set stage program counters
    setStagePCS();

    // Execution is finished if nops are in all stages except WB
    if (m_pcs.WB.first == m_textSize) {
        m_finished = true;
        return 1;
    } else {
        return 0;
    }
}

#define PCVAL(pc) std::pair<uint32_t, bool>((uint32_t)pc, true)
void Pipeline::setStagePCS() {
    // To validate a PC value (whether there is actually an instruction in the stage, or if the pipeline has
    // been reset), the previous stage PC is used to determine the current state of a stage To facilitate this,
    // the PCS are set in reverse order
    m_pcsPre = m_pcs;
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
    m_pcsPre.reset();
    m_finished = false;

    // Reset all registers to 0 and propagate signals through combinational logic
    RegBase::resetAll();
    propagateCombinational();

    // set PC_IF stage to the first instruction
    m_pcs.IF = PCVAL(r_PC_IF);
}
