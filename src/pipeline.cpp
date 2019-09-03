#include "pipeline.h"

#include "parser.h"

Pipeline::Pipeline() {
    // Connect pipeline

    // ------ IF ---------
    // Connect alu to PC and a constant "4" signal, and set operator to "Add"
    alu_pc4.setInputs(r_PC_IF.getOutput(), &c_4);
    alu_pc4.setControl(&s_alu_const_add);
    r_PC_IF.setInput(mux_PCSrc.getOutput());
    r_PC_IF.setEnable(&s_PCWrite);
    r_instr_IFID.setInput(&s_instr_IF);
    r_instr_IFID.setReset(&s_IFID_reset);
    r_instr_IFID.setEnable(&s_IFID_write);
    mux_PCSrc.setControl(&s_PCSrc);
    mux_PCSrc.setInput(0, alu_pc4.getOutput());
    mux_PCSrc.setInput(1, alu_pc_target.getOutput());
    mux_PCSrc.setInput(2, alu_mainALU.getOutput());

    r_PC_IFID.setInput(r_PC_IF.getOutput());
    r_PC_IFID.setReset(&s_IFID_reset);
    r_PC_IFID.setEnable(&s_IFID_write);
    r_invalidPC_IFID.setInput(&s_invalidPC);
    r_PC4_IFID.setInput(alu_pc4.getOutput());
    r_PC4_IFID.setReset(&s_IFID_reset);

    // ------ ID ---------
    m_reg.setInputs(&s_readRegister1, &s_readRegister2, r_writeReg_MEMWB.getOutput(), mux_memToReg.getOutput(),
                    r_regWrite_MEMWB.getOutput());
    r_imm_IDEX.setInput(&s_imm_ID);
    r_rd1_IDEX.setInput(m_reg.getOutput(1));
    r_rd2_IDEX.setInput(m_reg.getOutput(2));
    r_writeReg_IDEX.setInput(&writeReg);

    r_readRegister1_IDEX.setInput(&s_readRegister1);
    r_readRegister2_IDEX.setInput(&s_readRegister2);

    r_PC_IDEX.setInput(r_PC_IFID.getOutput());
    r_invalidPC_IDEX.setInput(r_invalidPC_IFID.getOutput());
    r_PC4_IDEX.setInput(r_PC4_IFID.getOutput());

    mux_forwardA_ID.setControl(&s_forwardA_ID);
    mux_forwardA_ID.setInput(Forwarding::NONE, m_reg.getOutput(1));
    mux_forwardA_ID.setInput(Forwarding::EXMEM, mux_alures_PC4_MEM.getOutput());
    mux_forwardA_ID.setInput(Forwarding::MEMWB, mux_memToReg.getOutput());

    mux_forwardB_ID.setControl(&s_forwardB_ID);
    mux_forwardB_ID.setInput(Forwarding::NONE, m_reg.getOutput(2));
    mux_forwardB_ID.setInput(Forwarding::EXMEM, mux_alures_PC4_MEM.getOutput());
    mux_forwardB_ID.setInput(Forwarding::MEMWB, mux_memToReg.getOutput());

    alu_pc_target.setControl(&s_alu_const_add);
    alu_pc_target.setInputs(r_PC_IFID.getOutput(), &s_imm_ID);

    // Control signals
    r_regWrite_IDEX.setInput(&s_RegWrite);
    r_ALUOP_IDEX.setInput(&s_ALUOP);
    r_ALUSrc1_IDEX.setInput(&s_ALUSrc1);
    r_ALUSrc2_IDEX.setInput(&s_ALUSrc2);
    r_MemWrite_IDEX.setInput(&s_MemWrite);
    r_MemRead_IDEX.setInput(&s_MemRead);
    r_memToReg_IDEX.setInput(&s_memToReg);
    r_jal_IDEX.setInput(&s_jal);
    r_jalr_IDEX.setInput(&s_jalr);

    // Gather all registers in a bank - making it easier to assign equal signals to all.
    // We do not include r_invalidPC_IDEX since this value is needed for the GUI to correctly identify flushed/stalled
    // stages
    bank_IDEX.addToBank(&r_imm_IDEX)
        .addToBank(&r_rd1_IDEX)
        .addToBank(&r_rd2_IDEX)
        .addToBank(&r_writeReg_IDEX)
        .addToBank(&r_readRegister1_IDEX)
        .addToBank(&r_readRegister2_IDEX)
        .addToBank(&r_PC_IDEX)
        .addToBank(&r_PC4_IDEX)
        .addToBank(&r_regWrite_IDEX)
        .addToBank(&r_ALUOP_IDEX)
        .addToBank(&r_ALUSrc1_IDEX)
        .addToBank(&r_ALUSrc2_IDEX)
        .addToBank(&r_MemWrite_IDEX)
        .addToBank(&r_MemRead_IDEX)
        .addToBank(&r_memToReg_IDEX)
        .addToBank(&r_jal_IDEX)
        .addToBank(&r_jal_IDEX);
    bank_IDEX.setReset(&s_IDEX_reset);
    bank_IDEX.setRegisterControls();

    // ------ EX ---------
    alu_mainALU.setInputs(mux_ALUSrc1.getOutput(), mux_ALUSrc2.getOutput());
    alu_mainALU.setControl(r_ALUOP_IDEX.getOutput());

    mux_ALUSrc1.setInput(0, mux_forwardA_EX.getOutput());
    mux_ALUSrc1.setInput(1, r_PC_IDEX.getOutput());
    mux_ALUSrc1.setControl(r_ALUSrc1_IDEX.getOutput());

    mux_ALUSrc2.setInput(0, mux_forwardB_EX.getOutput());
    mux_ALUSrc2.setInput(1, r_imm_IDEX.getOutput());
    mux_ALUSrc2.setControl(r_ALUSrc2_IDEX.getOutput());
    r_alures_EXMEM.setInput(alu_mainALU.getOutput());
    r_writeData_EXMEM.setInput(mux_forwardB_EX.getOutput());
    r_writeReg_EXMEM.setInput(r_writeReg_IDEX.getOutput());

    r_PC_EXMEM.setInput(r_PC_IDEX.getOutput());
    r_invalidPC_EXMEM.setInput(r_invalidPC_IDEX.getOutput());
    r_PC4_EXMEM.setInput(r_PC4_IDEX.getOutput());

    r_jal_EXMEM.setInput(r_jal_IDEX.getOutput());
    r_jalr_EXMEM.setInput(r_jalr_IDEX.getOutput());

    mux_forwardA_EX.setControl(&s_forwardA_EX);
    mux_forwardA_EX.setInput(Forwarding::NONE, r_rd1_IDEX.getOutput());
    mux_forwardA_EX.setInput(Forwarding::EXMEM, mux_alures_PC4_MEM.getOutput());
    mux_forwardA_EX.setInput(Forwarding::MEMWB, mux_memToReg.getOutput());

    mux_forwardB_EX.setControl(&s_forwardB_EX);
    mux_forwardB_EX.setInput(Forwarding::NONE, r_rd2_IDEX.getOutput());
    mux_forwardB_EX.setInput(Forwarding::EXMEM, mux_alures_PC4_MEM.getOutput());
    mux_forwardB_EX.setInput(Forwarding::MEMWB, mux_memToReg.getOutput());

    // Control signals
    r_regWrite_EXMEM.setInput(r_regWrite_IDEX.getOutput());
    r_MemWrite_EXMEM.setInput(r_MemWrite_IDEX.getOutput());
    r_MemRead_EXMEM.setInput(r_MemRead_IDEX.getOutput());
    r_memToReg_EXMEM.setInput(r_memToReg_IDEX.getOutput());

    // ------ MEM --------
    mux_alures_PC4_MEM.setInput(0, r_alures_EXMEM.getOutput());
    mux_alures_PC4_MEM.setInput(1, r_PC4_EXMEM.getOutput());
    mux_alures_PC4_MEM.setControl(&s_alures_PC4_MEM);

    r_readData_MEMWB.setInput(&readData_MEM);
    r_alures_MEMWB.setInput(r_alures_EXMEM.getOutput());
    r_writeReg_MEMWB.setInput(r_writeReg_EXMEM.getOutput());

    r_regWrite_MEMWB.setInput(r_regWrite_EXMEM.getOutput());
    r_memToReg_MEMWB.setInput(r_memToReg_EXMEM.getOutput());
    r_PC_MEMWB.setInput(r_PC_EXMEM.getOutput());
    r_invalidPC_MEMWB.setInput(r_invalidPC_EXMEM.getOutput());
    r_PC4_MEMWB.setInput(r_PC4_EXMEM.getOutput());

    // ------ WB ---------
    mux_memToReg.setInput(MemToReg::MEMREAD, r_readData_MEMWB.getOutput());
    mux_memToReg.setInput(MemToReg::ALURES, r_alures_MEMWB.getOutput());
    mux_memToReg.setInput(MemToReg::PC4, r_PC4_MEMWB.getOutput());
    mux_memToReg.setControl(r_memToReg_MEMWB.getOutput());
}

void Pipeline::immGen() {
    auto instr = (uint32_t)r_instr_IFID;
    uint32_t opcode = instr & 0b1111111;
    if (opcode == 0b0110111 || opcode == 0b0010111) {
        // LUI & AUIPC
        s_imm_ID = Signal<32>(instr & 0xfffff000);
    } else if (opcode == 0b1101111) {
        // JAL
        auto fields = Parser::getParser()->decodeJInstr(instr);
        s_imm_ID = signextend<int32_t, 21>(fields[0] << 20 | fields[1] << 1 | fields[2] << 11 | fields[3] << 12);
    } else if (opcode == 0b1100111) {
        // JALR
        s_imm_ID = Signal<32>(signextend<int32_t, 12>((instr >> 20)));
    } else {
        auto fields = Parser::getParser()->decodeRInstr(instr);
        if (opcode == 0b0010011 && (fields[3] == 0b001 || fields[3] == 0b101)) {
            // SLLI/SRLI/SRAI
            s_imm_ID = Signal<32>(fields[1]);
        } else {
            // Load, store or regular immediate instruction
            // Opcode bits 5 and 6 can define the required fields for generating the immediate
            switch ((instr & 0b1100000) >> 5) {
                case 0b00: {
                    // Load or regular immediate instruction, sign extend bits 31:20
                    s_imm_ID = Signal<32>(signextend<int32_t, 12>((instr >> 20)));
                    break;
                }
                case 0b01: {
                    // Store instruction
                    uint32_t v(r_instr_IFID);
                    s_imm_ID = Signal<32>(signextend<int32_t, 12>(((v & 0xfe000000)) >> 20) | ((v & 0xf80) >> 7));
                    break;
                }
                default: {
                    // Conditional branch instructions
                    auto fields = Parser::getParser()->decodeBInstr(instr);
                    s_imm_ID = signextend<int32_t, 13>((fields[0] << 12) | (fields[1] << 5) | (fields[5] << 1) |
                                                       (fields[6] << 11));
                    break;
                }
            }
        }
    }
}

void Pipeline::controlGen() {
    // Deassert all control lines
    s_ALUOP = 0;
    s_ALUSrc1 = 0;
    s_ALUSrc2 = 0;
    s_memToReg = MemToReg::ALURES;
    s_RegWrite = 0;
    s_MemRead = 0;
    s_MemWrite = 0;
    s_Branch = 0;
    s_CompOp = 0;
    s_jal = 0;
    s_jalr = 0;

    // Set control signals for the pipeline based on the input instruction
    switch ((uint32_t)r_instr_IFID & 0b1111111) {
        case 0b0110111: {
            // LUI
            s_ALUOP = ALUOps::LUI;
            s_ALUSrc2 = 1;
            s_RegWrite = 1;
            break;
        }
        case 0b0010111: {
            // AUIPC
            s_ALUSrc1 = 1;
            s_ALUOP = ALUOps::ADD;
            s_ALUSrc2 = 1;
            s_RegWrite = 1;
            break;
        }
        case 0b0010011: {
            // I-Type
            auto fields = Parser::getParser()->decodeIInstr((uint32_t)r_instr_IFID);
            switch (fields[2]) {
                case 0b000: {
                    // ADDI
                    s_ALUOP = ALUOps::ADD;
                    break;
                }
                case 0b010: {
                    // SLTI
                    s_ALUOP = ALUOps::LT;
                    break;
                }
                case 0b011: {
                    // SLTIU
                    s_ALUOP = ALUOps::LTU;
                    break;
                }
                case 0b100: {
                    // XORI
                    s_ALUOP = ALUOps::XOR;
                    break;
                }
                case 0b110: {
                    // ORI
                    s_ALUOP = ALUOps::OR;
                    break;
                }
                case 0b111: {
                    // ANDI
                    s_ALUOP = ALUOps::AND;
                    break;
                }
                case 0b001: {
                    // SLLI
                    s_ALUOP = ALUOps::SL;
                    break;
                }
                case 0b101: {
                    switch ((uint32_t)r_instr_IFID >> 25) {
                        case 0b0: {
                            // SRLI
                            s_ALUOP = ALUOps::SRL;
                            break;
                        }
                        case 0b0100000: {
                            // SRAI
                            s_ALUOP = ALUOps::SRA;
                            break;
                        }
                    }
                    break;
                }
            }
            s_ALUSrc2 = 1;
            s_RegWrite = 1;
            break;
        }
        case 0b0110011: {
            // R-type
            auto fields = Parser::getParser()->decodeRInstr((uint32_t)r_instr_IFID);
            if (fields[0] == 0b1) {
                // RV32M Standard extension
                switch (fields[3]) {
                    case 0b000: {
                        // mul
                        s_ALUOP = ALUOps::MUL;
                        break;
                    }
                    case 0b001: {
                        // mulh
                        s_ALUOP = ALUOps::MULH;
                        break;
                    }
                    case 0b010: {
                        // mulhsu
                        s_ALUOP = ALUOps::MULHSU;
                        break;
                    }
                    case 0b011: {
                        // mulhu
                        s_ALUOP = ALUOps::MULHU;
                        break;
                    }
                    case 0b100: {
                        // div
                        s_ALUOP = ALUOps::DIV;
                        break;
                    }
                    case 0b101: {
                        // divu
                        s_ALUOP = ALUOps::DIVU;
                        break;
                    }
                    case 0b110: {
                        // rem
                        s_ALUOP = ALUOps::REM;
                        break;
                    }
                    case 0b111: {
                        // remu
                        s_ALUOP = ALUOps::REMU;
                        break;
                    }
                }
            } else {
                switch (fields[3]) {
                    case 0b000: {
                        switch (fields[0]) {
                            case 0b0000000: {
                                // ADD
                                s_ALUOP = ALUOps::ADD;
                                break;
                            }
                            case 0b0100000: {
                                // SUB
                                s_ALUOP = ALUOps::SUB;
                                break;
                            }
                        }
                        break;
                    }
                    case 0b001: {
                        // SLL
                        s_ALUOP = ALUOps::SL;
                        break;
                    }
                    case 0b010: {
                        // SLT
                        s_ALUOP = ALUOps::LT;
                        break;
                    }
                    case 0b011: {
                        // SLTU
                        s_ALUOP = ALUOps::LTU;
                        break;
                    }
                    case 0b100: {
                        // XOR
                        s_ALUOP = ALUOps::XOR;
                        break;
                    }
                    case 0b101: {
                        switch (fields[0]) {
                            case 0b0000000: {
                                // SRL
                                s_ALUOP = ALUOps::SRL;
                                break;
                            }
                            case 0b0100000: {
                                // SRA
                                s_ALUOP = ALUOps::SRA;
                                break;
                            }
                        }
                        break;
                    }
                    case 0b110: {
                        // OR
                        s_ALUOP = ALUOps::OR;
                        break;
                    }
                    case 0b111: {
                        // AND
                        s_ALUOP = ALUOps::AND;
                        break;
                    }
                }
            }
            s_RegWrite = 1;
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
            s_ALUSrc2 = 1;
            s_memToReg = MemToReg::MEMREAD;
            s_RegWrite = 1;
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
            s_ALUSrc2 = 1;
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
            s_Branch = 1;
            break;
        }
        case 0b1101111: {
            // JAL
            s_RegWrite = 1;
            s_memToReg = MemToReg::PC4;
            s_jal = 1;
            break;
        }
        case 0b1100111: {
            // JALR
            s_ALUOP = ALUOps::ADD;
            s_ALUSrc2 = 1;
            s_memToReg = MemToReg::PC4;
            s_RegWrite = 1;
            s_jalr = 1;
        }
        default: {
            // Signals are already deasserted
            break;
        }
    }
}

namespace {
// Assert that the regWrite signal is asserted for a stage when checking forwarding, and that the destination registor
// is not 0. If this check is not done, coincidences between ie. SW immediate fields and rd fields can occur
#define EXMEM_WILL_WRITE (uint32_t) r_writeReg_EXMEM != 0 && (bool)r_regWrite_EXMEM
#define MEMWB_WILL_WRITE (uint32_t) r_writeReg_MEMWB != 0 && (bool)r_regWrite_MEMWB
#define TAS(sig, expr, value) \
    if (expr) {               \
        sig = value;          \
    }
}  // namespace

void Pipeline::forwardingControlGen() {
    // After clocking new values in all registers, examine register sources and assign forwading multiplexer control
    // signals accordingly

    // ----- ID stage ----- forwarding to register comparison logic
    // Register 1
    if ((uint32_t)s_readRegister1 == (uint32_t)r_writeReg_EXMEM && EXMEM_WILL_WRITE) {
        // Forward alures
        s_forwardA_ID = Forwarding::EXMEM;
    } else if ((uint32_t)s_readRegister1 == (uint32_t)r_writeReg_MEMWB && MEMWB_WILL_WRITE) {
        // Forward memread
        s_forwardA_ID = Forwarding::MEMWB;
    } else {
        s_forwardA_ID = Forwarding::NONE;
    }

    // Register 2
    if ((uint32_t)s_readRegister2 == (uint32_t)r_writeReg_EXMEM && EXMEM_WILL_WRITE) {
        s_forwardB_ID = Forwarding::EXMEM;
    } else if ((uint32_t)s_readRegister2 == (uint32_t)r_writeReg_MEMWB && MEMWB_WILL_WRITE) {
        s_forwardB_ID = Forwarding::MEMWB;
    } else {
        s_forwardB_ID = Forwarding::NONE;
    }

    // ----- EX stage ----- forwarding to ALU
    // Register 1
    if ((uint32_t)r_readRegister1_IDEX == (uint32_t)r_writeReg_EXMEM && EXMEM_WILL_WRITE) {
        s_forwardA_EX = Forwarding::EXMEM;
    } else if ((uint32_t)r_readRegister1_IDEX == (uint32_t)r_writeReg_MEMWB && MEMWB_WILL_WRITE) {
        s_forwardA_EX = Forwarding::MEMWB;
    } else {
        s_forwardA_EX = Forwarding::NONE;
    }

    // Register 2
    if ((uint32_t)r_readRegister2_IDEX == (uint32_t)r_writeReg_EXMEM && EXMEM_WILL_WRITE) {
        s_forwardB_EX = Forwarding::EXMEM;
    } else if ((uint32_t)r_readRegister2_IDEX == (uint32_t)r_writeReg_MEMWB && MEMWB_WILL_WRITE) {
        s_forwardB_EX = Forwarding::MEMWB;
    } else {
        s_forwardB_EX = Forwarding::NONE;
    }
}

void Pipeline::hazardControlGen() {
    // Some shorter names for readability
    auto r1 = (uint32_t)s_readRegister1;
    auto r2 = (uint32_t)s_readRegister2;

    if (r1 == 10 && r2 == 11) {
        volatile int a = 1;
        a++;
    }

    // Branch/ecall hazard: Result from EX stage is needed, or value from memory is needed.
    // This is BOTH for Branch and Ecall instructions, since these instructions work on operands forwarded
    // to the ID stage
    bool branchEcallHazardFromMem =
        ((r1 == (uint32_t)r_writeReg_EXMEM) || (r2 == (uint32_t)r_writeReg_EXMEM)) &&
        ((uint32_t)r_regWrite_EXMEM & 0b1) &&
        (((uint32_t)r_instr_IFID & 0b1111111) == 0b1100011 || ((uint32_t)r_instr_IFID & 0b1111111) == 0b1110011);

    bool branchEcallHazardFromEx =
        (r1 == (uint32_t)r_writeReg_IDEX || r2 == (uint32_t)r_writeReg_IDEX) && ((uint32_t)r_regWrite_IDEX & 0b1) &&
        (((uint32_t)r_instr_IFID & 0b1111111) == 0b1100011 || ((uint32_t)r_instr_IFID & 0b1111111) == 0b1110011);

    bool branchHazard = branchEcallHazardFromMem || branchEcallHazardFromEx;

    // Load Use hazard: Loaded variable is needed in execute stage
    bool loadUseHazard = (r1 == (uint32_t)r_writeReg_IDEX || r2 == (uint32_t)r_writeReg_IDEX) && (int)r_MemRead_IDEX;

    if (branchHazard || loadUseHazard) {
        // Stall until hazard is resolved - keep IFID and PC vaues, and reset IDEX registers
        s_PCWrite = 0;
        s_IFID_write = 0;
        s_IDEX_reset = 1;

    } else {
        s_PCWrite = 1;
        s_IFID_write = 1;
        s_IDEX_reset = 0;
    }

    // JALR hazard - IFID register is reset for s_jal & r_jal_idex but IDEX needs to be reset aswell for JALR
    if (r_jalr_IDEX) {
        s_IDEX_reset = 1;
    };
}

void Pipeline::propagateCombinational() {
    // The order of component updating must be done in the correct sequential order!
    // The rightmost stage is updated first, going from left to right in the given stage.
    // This is to ensure that feedback signals are valid

    // Extract register sources from current instruction
    if (((uint32_t)r_instr_IFID & 0b1111111) == 0b1110011) {
        // ECALL does not contain any operands, but we require that a0/a1 is
        // correctly forwarded. This is a bit hacky and actual processors does not handle
        // ECALL this way, but it works with the pipeline of Ripes
        s_readRegister1 = 10;
        s_readRegister2 = 11;
    } else {
        s_readRegister1 = (((uint32_t)r_instr_IFID) >> 15) & 0b11111;
        s_readRegister2 = (((uint32_t)r_instr_IFID) >> 20) & 0b11111;
    }

    // Do hazard detection and forwarding control generation
    forwardingControlGen();
    hazardControlGen();

    // ----- WB -----
    mux_memToReg.update();

    // ----- MEM -----
    s_alures_PC4_MEM = (uint32_t)(r_jalr_EXMEM || r_jal_EXMEM);
    mux_alures_PC4_MEM.update();
    if ((uint32_t)r_MemRead_EXMEM != 0) {
        // Store read access for use in GUI
        RVAccess acc{(uint32_t)r_PC_EXMEM, RW::Read, (uint32_t)r_alures_EXMEM, m_cycleCount};
        m_MemoryAccesses.insert(m_MemoryAccesses.begin(), acc);
        switch ((uint32_t)r_MemRead_EXMEM) {
            case LB: {
                readData_MEM = signextend<int32_t, 8>(m_runtimeMemory.read((uint32_t)r_alures_EXMEM) & 0xff);
                break;
            }
            case LH: {
                readData_MEM = signextend<int32_t, 16>(m_runtimeMemory.read((uint32_t)r_alures_EXMEM) & 0xffff);
                break;
            }
            case LW: {
                readData_MEM = m_runtimeMemory.read((uint32_t)r_alures_EXMEM);
                break;
            }
            case LBU: {
                readData_MEM = m_runtimeMemory.read((uint32_t)r_alures_EXMEM) & 0xff;
                break;
            }
            case LHU: {
                readData_MEM = m_runtimeMemory.read((uint32_t)r_alures_EXMEM) & 0xffff;
                break;
            }
        }
    }

    // ----- EX -----
    mux_forwardB_EX.update();
    mux_ALUSrc2.update();
    mux_forwardA_EX.update();
    mux_ALUSrc1.update();
    alu_mainALU.update();

    // ----- ID -----
    controlGen();
    immGen();
    alu_pc_target.update();
    m_reg.update();
    writeReg = Signal<5>(((uint32_t)r_instr_IFID >> 7) & 0b11111);
    mux_forwardA_ID.update();
    mux_forwardB_ID.update();

    // Compare read register values and '&' with s_branch control signal
    switch ((CompOp)(uint32_t)s_CompOp) {
        case BEQ: {
            s_branchTaken = s_Branch && ((uint32_t)mux_forwardA_ID == (uint32_t)mux_forwardB_ID);
            break;
        }
        case BNE: {
            s_branchTaken = s_Branch && ((uint32_t)mux_forwardA_ID != (uint32_t)mux_forwardB_ID);
            break;
        }
        case BLT: {
            s_branchTaken = s_Branch && ((int32_t)mux_forwardA_ID < (int32_t)mux_forwardB_ID);
            break;
        }
        case BLTU: {
            s_branchTaken = s_Branch && ((uint32_t)mux_forwardA_ID < (uint32_t)mux_forwardB_ID);
            break;
        }
        case BGE: {
            s_branchTaken = s_Branch && ((int32_t)mux_forwardA_ID >= (int32_t)mux_forwardB_ID);
            break;
        }
        case BGEU: {
            s_branchTaken = s_Branch && ((uint32_t)mux_forwardA_ID >= (uint32_t)mux_forwardB_ID);
            break;
        }
        default: { s_branchTaken = 0; }
    }

    // bit 1 =true for s_PCSrc will unconditionally take r_jal_IDEX PC target. Else, take branch outcome calculation if
    // s_branchTaken or s_jal. if deasserted, PC+4 is selected
    if (r_jalr_IDEX) {
        s_PCSrc = PCSRC::JALR;
    } else if ((uint32_t)s_branchTaken || s_jal) {
        s_PCSrc = PCSRC::BR;
    } else {
        s_PCSrc = PCSRC::PC4;
    }

    // IFID should be reset when branch is taken and when jumping
    s_IFID_reset = s_branchTaken || r_jalr_IDEX || s_jal;

    // ----- IF -----
    alu_pc4.update();
    mux_PCSrc.update();

    // Load nops if PC is greater than text size
    m_finishing = ((uint32_t)r_PC_IFID > m_textSize) | m_finishing;
    s_instr_IF =
        Signal<32>(m_finishing ? 0 : m_runtimeMemory.read((uint32_t)r_PC_IF));  // Read instruction at current PC

    // For GUI - set invalidPC (branch taken indicator) if  PCSrc both PCSrc and s_IFID_write is asserted - in this
    // case, a new program counter value is starting to propagate, indicating an invalid ID branch
    s_invalidPC = ((bool)s_branchTaken && (bool)s_IFID_write) || s_jal;
    TAS(s_invalidPC, (uint32_t)r_PC_IFID > m_textSize, HazardReason::eof);
    if (s_IDEX_reset) {
        // ID has a dependancy and requires a stall of EX. Set IDEX s_invalidPC register accordingly
        r_invalidPC_IDEX.overrideNext(HazardReason::STALL);
    }

    TAS(s_PCWrite, m_finishing, 0);
    TAS(s_IFID_reset, m_finishing, 1);
    // if finishing, ECALL will stay in ID stage, and we will force overrideNext for idex stage
    if (m_finishing) {
        r_invalidPC_IFID.overrideNext(HazardReason::eof);
    }
}

void Pipeline::handleEcall() {
    if (((uint32_t)r_instr_IFID & 0b1111111) == 0b1110011 &&
        static_cast<bool>(s_IFID_write)  // For checking load-use hazard
    ) {
        // ecall in EX stage
        auto a0 = (uint32_t)mux_forwardA_ID;
        auto a1 = (int32_t)mux_forwardB_ID;

        m_ecallArg = static_cast<ECALL>(a0);
        m_ecallVal = a1;

        if (m_ecallArg == ECALL::exit) {
            m_finishing = true;
        }
    }
}

std::pair<Pipeline::ECALL, int32_t> Pipeline::checkEcall(bool reset) {
    // Called from GUI after each clock cycle - resets the internal ECALL request
    const auto val = m_ecallArg;
    if (reset)
        m_ecallArg = ECALL::none;
    return {val, m_ecallVal};
}

void Pipeline::doFinishCleanup() {
    for (auto& reg : RegBase::registers) {
        reg->resetAll();
    }

    m_pcs.IF.invalidReason = 3;
    m_pcs.ID.invalidReason = 3;
    m_pcs.EX.invalidReason = 3;
    m_pcs.MEM.invalidReason = 3;
    m_pcs.WB.invalidReason = 3;
}

int Pipeline::step() {
    // Main processing function for the pipeline
    // propagates the signals throgh the combinational logic and clocks the
    // sequential logic afterwards - similar to clocking a circuit

    // We define an instruction as being when the PC of the WB stage is valid (initialized, non-stall/flush)
    if (m_pcs.WB.isValid() && m_pcs.WB.initialized) {
        m_instructionsExecuted++;
    }

    // Clock inter-stage registers
    m_reg.clock();

    m_cycleCount++;

    if ((uint32_t)r_MemWrite_EXMEM != 0) {
        // Store write access for use in GUI. Cycle count is current cycle count - 1 since the operation was in the MEM
        // stage in the previous cycle (which we should convey to the user), but the memory gets stored on the rising
        // edge of the clock (this cycle).
        RVAccess acc{(uint32_t)r_PC_EXMEM, RW::Write, (uint32_t)r_alures_EXMEM, m_cycleCount - 1};
        m_MemoryAccesses.insert(m_MemoryAccesses.begin(), acc);
        switch ((uint32_t)r_MemWrite_EXMEM) {
            case SB: {
                m_runtimeMemory.write((uint32_t)r_alures_EXMEM, (uint32_t)r_writeData_EXMEM, 1);
                break;
            }
            case SH: {
                m_runtimeMemory.write((uint32_t)r_alures_EXMEM, (uint32_t)r_writeData_EXMEM, 2);
                break;
            }
            case SW: {
                m_runtimeMemory.write((uint32_t)r_alures_EXMEM, (uint32_t)r_writeData_EXMEM, 4);
                break;
            }
        }
    }

    // handle ECALL I/O. If a0 = 10 for an ECALL, this sets m_finishing and increments finishing counter
    // if m_finishing is set, we disable PC writing
    handleEcall();

    // Clock stage-separating registers
    RegBase::clockAll();

    // Propagate signals through logic
    propagateCombinational();
    // Set stage program counters
    setStagePCS();

    // Store program counter for the current cycle
    m_pcsCycles.push_back(m_pcs);

    // Check for finished execution(either end of file or m_finishingCnt > 4 (if ecall 10 has been called)) and
    // breakpoints
    if (m_finishing) {
        m_finishingCnt++;
    }
    if ((m_pcs.WB.pc > m_textSize) || (m_finishingCnt > 4) || m_ecallArg == ECALL::exit) {
        m_finished = true;
        doFinishCleanup();
        return 1;
    } else if (m_breakpoints.find((uint32_t)r_PC_IF) != m_breakpoints.end()) {
        // Breakpoint set at current r_PC_IF value
        return 1;
    } else if (m_ecallArg != ECALL::none) {
        return 1;  // GUI will automatically resume execution if in "running" mode
    } else {
        return 0;
    }
}

#define PCVAL(pc, reg) \
    StagePCS::PC { (uint32_t) pc, (uint32_t)pc < m_textSize, (uint32_t)reg }

void Pipeline::setStagePCS() {
    // To validate a PC value (whether there is actually an instruction in the stage, or if the pipeline has
    // been reset), the previous stage PC is used to determine the current state of a stage To facilitate this,
    // the PCS are set in reverse order
    m_pcsPre = m_pcs;
    // Propagate "valid" in terms
    m_pcs.WB = m_pcs.MEM.initialized ? PCVAL(r_PC_MEMWB, r_invalidPC_MEMWB) : m_pcs.WB;
    m_pcs.MEM = m_pcs.EX.initialized ? PCVAL(r_PC_EXMEM, r_invalidPC_EXMEM) : m_pcs.MEM;
    m_pcs.EX = m_pcs.ID.initialized ? PCVAL(r_PC_IDEX, r_invalidPC_IDEX) : m_pcs.EX;
    m_pcs.ID = m_pcs.IF.initialized | m_finishing ? PCVAL(r_PC_IFID, r_invalidPC_IFID) : m_pcs.ID;
    m_pcs.IF = PCVAL(r_PC_IF, m_finishing | m_finished);
}

void Pipeline::abort() {
    m_abort = true;  // async abort of run()
}
void Pipeline::clearAbort() {
    m_abort = false;
}

int Pipeline::run() {
    m_running = true;
    while (!step() && !m_abort)
        ;
    m_running = false;
    if (m_abort) {
        restart();
        return 1;
    } else if (m_ecallArg != ECALL::none) {
        return 1;
    } else {
        return 0;
    }
}

void Pipeline::reset() {
    // Called when resetting the simulator (loading a new program)
    restart();
    m_runtimeMemory.clear();
    m_dataMemory.clear();
    m_baselineMemory.clear();
    m_textSize = 0;
    m_ready = false;
    m_abort = false;
}

void Pipeline::update() {
    // Must be called whenever external changes to the memory has been envoked
    m_ready = true;
    restart();
}

void Pipeline::restart() {
    // Called when restarting a simulation
    m_reg.clear();
    m_runtimeMemory = m_baselineMemory;
    m_textSize = m_runtimeMemory.size();
    m_runtimeMemory.insert(m_dataMemory.begin(),
                           m_dataMemory.end());  // Merge assembler-provided data memory into simulator memory
    m_reg.init();
    m_pcs.reset();
    m_pcsPre.reset();
    m_finished = false;
    m_finishing = false;
    m_abort = false;
    m_finishingCnt = 0;
    m_instructionsExecuted = 0;
    m_cycleCount = 0;
    m_ecallArg = ECALL::none;
    m_ecallVal = 0;
    m_pcsCycles.clear();
    m_MemoryAccesses.clear();

    // Reset all registers to 0 and propagate signals through combinational logic
    RegBase::resetAll();
    propagateCombinational();

    // set PC_IF stage to the first instruction
    m_pcs.IF = PCVAL(r_PC_IF, s_branchTaken);
    m_pcsCycles.push_back(m_pcs);
}
