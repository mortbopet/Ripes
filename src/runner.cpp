#include "runner.h"

#include "parser.h"

Runner::Runner(Parser *parser) {
  m_parser = parser;

  // memory allocation
  // temporary allocation method - get filesize from parser, and allocate same
  // amount of memory
  m_textSize = m_parser->getFileSize();
  m_text = new uint8_t[m_textSize];

  // file parsing
  m_parser->parseFile(m_text);
}

Runner::~Runner() {}

int Runner::exec() {
  // Main simulator loop:
  // Make parser parse an instruction based on the current program counter, and,
  // if successfull, execute the read instruction. Loop until parser
  // unsuccesfully parses an instruction.
  error err;
  while (getInstruction(m_pc)) {
    if ((err = execInstruction(m_currentInstruction)) != SUCCESS) {
      handleError(err);
    }
  }
  return 0;
}

bool Runner::getInstruction(int pc) {
  if (pc < m_textSize) {
    auto word = *((uint32_t *)m_text + pc);
    m_currentInstruction.word = word;
    m_currentInstruction.type = static_cast<instrType>(word & 0x7f);
    return true;
  }
  return false;
}

error Runner::execInstruction(Instruction instr) {
  switch (instr.type) {
  case LUI:
    return execLuiInstr(instr);
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
  default:
    return error::EXEC_ERR;
    break;
  }
}

error Runner::execLuiInstr(Instruction instr) {
  /* "LUI places the U-immediate value in the top 20 bits of
   * the destination m_register rd, filling in the lowest 12 bits with zeros"*/
  std::vector<uint32_t> fields = decodeUInstr(instr);
  m_reg[fields[1]] = fields[0] << 12;
  return SUCCESS;
}

error Runner::execJalInstr(Instruction instr) {
  std::vector<uint32_t> fields = decodeJInstr(instr);
  m_pc += fields[0] << 20 | fields[1] << 1 | fields[2] << 11 |
          fields[3] << 12; // must be signed!
  m_reg[fields[4]] =
      m_pc +
      4; // rd = pc + 4 // is rd equal to pc+4 before or after pc increment?
  return SUCCESS;
}

error Runner::execJalrInstr(Instruction instr) {
  std::vector<uint32_t> fields = decodeIInstr(instr);
  m_reg[fields[3]] = m_pc + 4; // store return address
  m_pc = ((int32_t)fields[0] + fields[1]) &
         0xfffe; // set LSB of result to zero // shouldnt this be 0xfffffffe?
  return SUCCESS;
}

error Runner::execBranchInstr(Instruction instr) {
  std::vector<uint32_t> fields = decodeBInstr(instr);

  // calculate target address using signed offset
  auto target = m_pc + (int32_t)((fields[0] << 12) | (fields[1] << 5) |
                                 (fields[5] << 1) | (fields[6] << 11));
  switch (fields[4]) {
  case 0b000: // BEQ
    m_pc = m_reg[fields[2]] == m_reg[fields[3]] ? target : m_pc + 4;
    break;
  case 0b001: // BNE
    m_pc = m_reg[fields[2]] != m_reg[fields[3]] ? target : m_pc + 4;
    break;
  case 0b100: // BLT - signed comparison
    m_pc = (int32_t)m_reg[fields[2]] > (int32_t)m_reg[fields[3]] ? target
                                                                 : m_pc + 4;
    break;
  case 0b101: // BGE - signed comparison
    m_pc = (int32_t)m_reg[fields[2]] <= (int32_t)m_reg[fields[3]] ? target
                                                                  : m_pc + 4;
    break;
  case 0b110: // BLTU
    m_pc = m_reg[fields[2]] > m_reg[fields[3]] ? target : m_pc + 4;
    break;
  case 0b111: // BGEU
    m_pc = m_reg[fields[2]] <= m_reg[fields[3]] ? target : m_pc + 4;
    break;
  default:
    return ERR_BFUNCT3;
  }
  return SUCCESS;
}

error Runner::execLoadInstr(Instruction instr) {
  std::vector<uint32_t> fields = decodeIInstr(instr);
  if (fields[3] == 0) {
    return ERR_NULLLOAD;
  }
  auto target = (int32_t)fields[0] + fields[1];

  // Handle different load types by pointer casting and subsequent
  // dereferencing. This will handle whether to sign or zero extend.
  switch (fields[2]) {
  case 0b000: // LB - load sign extended byte
    m_reg[fields[3]] = ((int8_t *)m_mem)[target];
    break;
  case 0b001: // LH load sign extended halfword
    m_reg[fields[3]] = ((int16_t *)m_mem)[target];
    break;
  case 0b010: // LW load word
    m_reg[fields[3]] = ((uint32_t *)m_mem)[target];
    break;
  case 0b100: // LBU load zero extended byte
    m_reg[fields[3]] = ((uint8_t *)m_mem)[target];
    break;
  case 0b101: // LHU load zero extended halfword
    m_reg[fields[3]] = ((uint16_t *)m_mem)[target];
    break;
  }
  return SUCCESS;
}

error Runner::execStoreInstr(Instruction instr) {
  std::vector<uint32_t> fields = decodeSInstr(instr);

  auto target = (int32_t)(fields[0] << 5 | fields[4]) + m_reg[fields[2]];
  switch (fields[3]) {
  case 0b000: // SB
    m_mem[target] = (uint8_t)m_reg[fields[1]];
    break;
  case 0b001:                                   // SH
    m_mem[target] = (uint16_t)m_reg[fields[1]]; // will this work? m_mem[target]
                                                // is uint8_t. I think we need
                                                // (uint16_t)((m_mem +
                                                // target)*), not sure though
    break;
  case 0b010: // SW
    m_mem[target] = m_reg[fields[1]];
    break;
  default:
    return ERR_BFUNCT3;
  }
  return SUCCESS;
}

error Runner::execOpImmInstr(Instruction instr) {
  std::vector<uint32_t> fields = decodeIInstr(instr);
  if (fields[3] == 0) {
    return ERR_NULLLOAD;
  }

  switch (fields[2]) {
  case 0b000: // ADDI
    m_reg[fields[3]] = m_reg[fields[1]] + (int32_t)fields[0];
    break;
  case 0b010: // SLTI
    m_reg[fields[3]] = m_reg[fields[1]] < (int32_t)fields[0] ? 1 : 0;
    break;
  case 0b011: // SLTIU
    m_reg[fields[3]] = m_reg[fields[1]] < fields[0] ? 1 : 0;
    break;
  case 0b100: // XORI
    m_reg[fields[3]] = m_reg[fields[1]] ^ fields[0];
    break;
  case 0b110: // ORI
    m_reg[fields[3]] = m_reg[fields[1]] | fields[0];
    break;
  case 0b111: // ANDI
    m_reg[fields[3]] = m_reg[fields[1]] & fields[0];
    break;
  }
  return SUCCESS;
}

error Runner::execOpInstr(Instruction instr) {
  std::vector<uint32_t> fields = decodeRInstr(instr);
  switch (fields[3]) {}
  // code stuff here
  return SUCCESS;
}

void Runner::handleError(error err) const {
  // handle error and print program counter + current instruction
}

std::vector<uint32_t> Runner::decodeUInstr(Instruction instr) const {
  std::vector<uint32_t> tmp;
  return tmp;
}
std::vector<uint32_t> Runner::decodeJInstr(Instruction instr) const {
  std::vector<uint32_t> tmp;
  return tmp;
}
std::vector<uint32_t> Runner::decodeIInstr(Instruction instr) const {
  std::vector<uint32_t> tmp;
  return tmp;
}
std::vector<uint32_t> Runner::decodeSInstr(Instruction instr) const {
  std::vector<uint32_t> tmp;
  return tmp;
}
std::vector<uint32_t> Runner::decodeRInstr(Instruction instr) const {
  std::vector<uint32_t> tmp;
  return tmp;
}
std::vector<uint32_t> Runner::decodeBInstr(Instruction instr) const {
  std::vector<uint32_t> tmp;
  return tmp;
}
