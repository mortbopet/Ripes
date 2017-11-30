#include "runner.h"
#include "assert.h"
#include "parser.h"

#include "binutils.h"

Runner::Runner(Parser* parser) {
    m_parser = parser;

    // memory allocation
    // temporary allocation method - get filesize from parser, and allocate same
    // amount of memory
    m_reg = std::vector<uint32_t>(32, 0);

    // Set default register values
    m_reg[0] = 0;
    m_reg[2] = m_stackStart;
    m_reg[3] = m_dataStart;

    // file parsing
    m_parser->parseFile(&m_memory);
    // Assuming the map was empty when parsing the program into memory, the size of the memory will now equate to the
    // number of instructions in the text segment
    m_textSize = m_memory.size();
}

Runner::~Runner() {}

int Runner::exec() {
    // Main simulator loop:
    // Make parser parse an instruction based on the current program counter, and,
    // if successfull, execute the read instruction. Loop until parser
    // unsuccesfully parses an instruction.
    instrState err;
    while (getInstruction(m_pc)) {
        switch ((err = execInstruction(m_currentInstruction))) {
            case SUCCESS:
                break;
            case DONE:
                return 0;
            default:
                handleError(err);
                return 1;
        }
    }
    return 0;
}

bool Runner::getInstruction(int pc) {
    uint32_t word;
    word = m_memory[pc] + (m_memory[pc + 1] << 8) + (m_memory[pc + 2] << 16) + (m_memory[pc + 3] << 24);

    m_currentInstruction.word = word;
    m_currentInstruction.type = static_cast<instrType>(word & 0x7f);
    return true;
}

instrState Runner::execInstruction(Instruction instr) {
    switch (instr.type) {
        case LUI:
            return execLuiInstr(instr);
        case AUIPC:
            return execAuipcInstr(instr);
        case JAL:
            return execJalInstr(instr);
        case JALR:
            return execJalrInstr(instr);
        case BRANCH:
            return execBranchInstr(instr);
        case LOAD:
            return execLoadInstr(instr);
        case STORE:
            return execStoreInstr(instr);
        case OP_IMM:
            return execOpImmInstr(instr);
        case OP:
            return execOpInstr(instr);
        case ECALL:
            return execEcallInstr();
        default:
            return instrState::EXEC_ERR;
            break;
    }
}

instrState Runner::execLuiInstr(Instruction instr) {
    /* "LUI places the U-immediate value in the top 20 bits of
     * the destination m_register rd, filling in the lowest 12 bits with zeros"*/
    std::vector<uint32_t> fields = m_parser->decodeUInstr(instr.word);
    m_reg[fields[1]] = fields[0] << 12;
    m_pc += 4;
    return SUCCESS;
}

instrState Runner::execAuipcInstr(Instruction instr) {
    std::vector<uint32_t> fields = m_parser->decodeUInstr(instr.word);
    m_reg[fields[1]] = (fields[0] << 12) + m_pc;
    m_pc += 4;
    return SUCCESS;
}

instrState Runner::execJalInstr(Instruction instr) {
    std::vector<uint32_t> fields = m_parser->decodeJInstr(instr.word);
    if (fields[4] != 0) {  // rd = 0 equals unconditional jump
        m_reg[fields[4]] = m_pc + 4;
    }
    m_pc += signextend<int32_t, 21>(fields[0] << 20 | fields[1] << 1 | fields[2] << 11 | fields[3] << 12);
    // Check for misaligned four-byte boundary
    if ((m_pc & 0b11) != 0) {
        return EXEC_ERR;
    } else {
        return SUCCESS;
    }
}

instrState Runner::execJalrInstr(Instruction instr) {
    std::vector<uint32_t> fields = m_parser->decodeIInstr(instr.word);
    // Store initial register state of m_reg[fields[1]], in case
    // fields[3]==fields[1]
    uint32_t reg1 = m_reg[fields[1]];

    if (fields[3] != 0) {             // if rd = 0, dont store address
        m_reg[fields[3]] = m_pc + 4;  // store return address
    }
    m_pc = (signextend<int32_t, 12>(fields[0]) + reg1) & 0xfffffffe;  // set LSB of result to zero

    // Check for misaligned four-byte boundary
    if ((m_pc & 0b11) != 0) {
        return EXEC_ERR;
    } else {
        return SUCCESS;
    }
}

instrState Runner::execBranchInstr(Instruction instr) {
    std::vector<uint32_t> fields = m_parser->decodeBInstr(instr.word);

    // calculate target address using signed offset
    uint32_t target =
        m_pc + signextend<int32_t, 13>((fields[0] << 12) | (fields[1] << 5) | (fields[5] << 1) | (fields[6] << 11));
    switch (fields[4]) {
        case 0b000:  // BEQ
            m_pc = m_reg[fields[2]] == m_reg[fields[3]] ? target : m_pc + 4;
            break;
        case 0b001:  // BNE
            m_pc = m_reg[fields[2]] != m_reg[fields[3]] ? target : m_pc + 4;
            break;
        case 0b100:  // BLT - signed comparison
            m_pc = (int32_t)m_reg[fields[3]] < (int32_t)m_reg[fields[2]] ? target : m_pc + 4;
            break;
        case 0b101:  // BGE - signed comparison
            m_pc = (int32_t)m_reg[fields[3]] >= (int32_t)m_reg[fields[2]] ? target : m_pc + 4;
            break;
        case 0b110:  // BLTU
            m_pc = m_reg[fields[3]] < m_reg[fields[2]] ? target : m_pc + 4;
            break;
        case 0b111:  // BGEU
            m_pc = m_reg[fields[3]] >= m_reg[fields[2]] ? target : m_pc + 4;
            break;
        default:
            return ERR_BFUNCT3;
    }
    return SUCCESS;
}

instrState Runner::execLoadInstr(Instruction instr) {
    std::vector<uint32_t> fields = m_parser->decodeIInstr(instr.word);
    if (fields[3] == 0) {
        return ERR_NULLLOAD;
    }
    uint32_t target = signextend<int32_t, 12>(fields[0]) + m_reg[fields[1]];

    // Handle different load types by pointer casting and subsequent
    // dereferencing. This will handle whether to sign or zero extend.
    switch (fields[2]) {
        case 0b000:  // LB - load sign extended byte
            m_reg[fields[3]] = signextend<int32_t, 8>(memRead(target));
            break;
        case 0b001:  // LH load sign extended halfword
            m_reg[fields[3]] = signextend<int32_t, 16>(memRead(target));
            break;
        case 0b010:  // LW load word
            m_reg[fields[3]] = memRead(target);
            break;
        case 0b100:  // LBU load zero extended byte
            m_reg[fields[3]] = memRead(target) & 0x000000ff;
            break;
        case 0b101:  // LHU load zero extended halfword
            m_reg[fields[3]] = memRead(target) & 0x0000ffff;
            break;
    }
    m_pc += 4;
    return SUCCESS;
}

void Runner::memWrite(uint32_t address, uint32_t value, int size) {
    // writes value to from the given address start, and up to $size bytes of
    // $value
    // Using the hashtable, new allocations will automatically be handled
    for (int i = 0; i < size; i++) {
        m_memory[address + i] = value & 0xff;
        value >>= 8;
    }
}

uint32_t Runner::memRead(uint32_t address) {
    // Note: If address is not found in memory map, a default constructed object
    // will be created, and read. in our case uint8_t() = 0
    uint32_t read = (m_memory[address] | (m_memory[address + 1] << 8) | (m_memory[address + 2] << 16) |
                     (m_memory[address + 3] << 24));
    return read;
}

instrState Runner::execStoreInstr(Instruction instr) {
    std::vector<uint32_t> fields = m_parser->decodeSInstr(instr.word);

    auto target = signextend<int32_t, 12>((fields[0] << 5) | fields[4]) + m_reg[fields[2]];
    switch (fields[3]) {
        case 0b000:  // SB
            memWrite(target, m_reg[fields[1]] & 0x000000ff, 1);
            break;
        case 0b001:  // SH
            memWrite(target, m_reg[fields[1]] & 0x0000ffff, 2);
            break;
        case 0b010:  // SW
            memWrite(target, m_reg[fields[1]], 4);
            break;
        default:
            return ERR_BFUNCT3;
    }
    m_pc += 4;
    return SUCCESS;
}

instrState Runner::execOpImmInstr(Instruction instr) {
    std::vector<uint32_t> fields = m_parser->decodeIInstr(instr.word);
    if (fields[3] == 0) {
        return ERR_NULLLOAD;
    }

    switch (fields[2]) {
        case 0b000:  // ADDI
            m_reg[fields[3]] = (int32_t)m_reg[fields[1]] + signextend<int32_t, 12>(fields[0]);
            break;
        case 0b001:  // SLLI
            m_reg[fields[3]] = m_reg[fields[1]] << (fields[0] & 0b11111);
            break;
        case 0b010:  // SLTI
            m_reg[fields[3]] = (int32_t)m_reg[fields[1]] < signextend<int32_t, 12>(fields[0]) ? 1 : 0;
            break;
        case 0b011:  // SLTIU
            m_reg[fields[3]] = m_reg[fields[1]] < fields[0] ? 1 : 0;
            break;
        case 0b100:  // XORI
            m_reg[fields[3]] = m_reg[fields[1]] ^ fields[0];
            break;
        case 0b101:
            if ((fields[0] >> 5) == 0) {
                m_reg[fields[3]] = m_reg[fields[1]] >> (fields[0] & 0b11111);  // SRLI
                break;
            } else if ((fields[0] >> 5) == 0b0100000) {  // SRAI
                m_reg[fields[3]] = (int32_t)m_reg[fields[1]] >> (fields[0] & 0b11111);
                break;
            } else {
                return EXEC_ERR;
            }
        case 0b110:  // ORI
            m_reg[fields[3]] = m_reg[fields[1]] | fields[0];
            break;
        case 0b111:  // ANDI
            m_reg[fields[3]] = m_reg[fields[1]] & fields[0];
            break;
    }
    m_pc += 4;
    return SUCCESS;
}

instrState Runner::execOpInstr(Instruction instr) {
    std::vector<uint32_t> fields = m_parser->decodeRInstr(instr.word);
    switch (fields[3]) {
        case 0b000:
            if (fields[0] == 0) {
                m_reg[fields[4]] = (int32_t)m_reg[fields[2]] + (int32_t)m_reg[fields[1]];  // ADD
                break;
            } else if (fields[0] == 0b0100000) {
                m_reg[fields[4]] = (int32_t)m_reg[fields[2]] - (int32_t)m_reg[fields[1]];  // SUB
                break;
            } else if (fields[0] == 0b0000001) {
                // MUL
                m_reg[fields[4]] = (uint32_t)m_reg[fields[1]] * (uint32_t)m_reg[fields[2]];
                break;
            }
        case 0b001:
            if (fields[0] == 0b0000001) {
                // MULH
                int64_t res = signextend<int64_t, 32>(m_reg[fields[1]]) * signextend<int64_t, 32>(m_reg[fields[2]]);
                res >>= 32;
                m_reg[fields[4]] = res;
                break;
            } else if (fields[0] == 0) {
                // SLL
                m_reg[fields[4]] = m_reg[fields[2]] << (m_reg[fields[1]] & 0x1F);
                break;
            }
        case 0b010:
            if (fields[0] == 0b0000001) {
                // MULHSU
                int64_t res = m_reg[fields[1]] * signextend<int64_t, 32>(m_reg[fields[2]]);
                res >>= 32;
                m_reg[fields[4]] = res;
                break;
            } else if (fields[0] == 0) {
                // SLT
                m_reg[fields[4]] = (int32_t)m_reg[fields[2]] < (int32_t)m_reg[fields[1]] ? 1 : 0;
                break;
            }
        case 0b011:
            if (fields[0] == 0b0000001) {
                // MULHU
                uint64_t res = m_reg[fields[2]] * m_reg[fields[1]];
                res >>= 32;
                m_reg[fields[4]] = res;
                break;
            } else if (fields[2] == 0) {
                // SLTU
                m_reg[fields[4]] = m_reg[fields[1]] != 0 ? 1 : 0;
                break;
            } else {
                // SLTU
                m_reg[fields[4]] = m_reg[fields[2]] < m_reg[fields[1]] ? 1 : 0;
                break;
            }
        case 0b100:
            if (fields[0] == 0b1) {
                // DIV
                if (m_reg[fields[1]] == 0) {
                    // Divison by zero
                    m_reg[fields[4]] = -1;
                    break;
                } else if ((int32_t)m_reg[fields[1]] == -1 && (int32_t)m_reg[fields[2]] == -pow(-2, 31)) {
                    // Overflow
                    m_reg[fields[4]] = -pow(2, 31);
                    break;
                } else {
                    m_reg[fields[4]] = (int32_t)m_reg[fields[2]] / (int32_t)m_reg[fields[1]];
                    break;
                }
            } else {
                // XOR
                m_reg[fields[4]] = m_reg[fields[2]] ^ m_reg[fields[1]];
                break;
            }
        case 0b101:  // SRL and SRA
            if (fields[0] == 0) {
                m_reg[fields[4]] = m_reg[fields[2]] >> (m_reg[fields[1]] & 0x1F);  // SRL
                break;
            } else if (fields[0] == 0b0100000) {
                m_reg[fields[4]] =  // Cast to signed when doing arithmetic shift
                    (int32_t)m_reg[fields[2]] >> (m_reg[fields[1]] & 0x1F);  // SRA
                break;
            } else if (fields[0] == 0b1) {
                // DIVU
                if (m_reg[fields[1]] == 0) {
                    // Divison by zero
                    m_reg[fields[4]] = pow(2, 31) - 1;
                    break;
                } else {
                    m_reg[fields[4]] = m_reg[fields[2]] / m_reg[fields[1]];
                    break;
                }
            } else {
                return EXEC_ERR;
            }
        case 0b110:
            if (fields[0] == 0b1) {
                // REM
                if (m_reg[fields[1]] == 0) {
                    // Divison by zero
                    m_reg[fields[4]] = m_reg[fields[2]];
                    break;
                } else if ((int32_t)m_reg[fields[1]] == -1 && (int32_t)m_reg[fields[2]] == -pow(-2, 31)) {
                    // Overflow
                    m_reg[fields[4]] = 0;
                    break;
                } else {
                    m_reg[fields[4]] = (int32_t)m_reg[fields[2]] % (int32_t)m_reg[fields[1]];
                    break;
                }
            } else {
                // OR
                m_reg[fields[4]] = m_reg[fields[2]] | m_reg[fields[1]];
                break;
            }
        case 0b111:
            if (fields[0] == 0b1) {
                // REMU
                if (m_reg[fields[1]] == 0) {
                    // Divison by zero
                    m_reg[fields[4]] = m_reg[fields[2]];
                    break;
                } else {
                    m_reg[fields[4]] = m_reg[fields[2]] % m_reg[fields[1]];
                    break;
                }
            } else {
                // AND
                m_reg[fields[4]] = m_reg[fields[2]] & m_reg[fields[1]];
                break;
            }
    }

    m_pc += 4;
    return SUCCESS;
}

instrState Runner::execEcallInstr() {
    switch (m_reg[10])  // a0
    {
        case 10:
            return DONE;
        default:
            return DONE;
    }
}

void Runner::handleError(instrState /*err*/) const {
    // handle error and print program counter + current instruction
    throw "Error!";
}
