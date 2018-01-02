#include "pipelineobjects.h"

// Instantiate register vector
std::vector<RegBase*> RegBase::registers;

void Registers::update() {
    // Read
    int readRegister1 = (((uint32_t)*m_instr) >> 15) & 0b11111;
    int readRegister2 = (((uint32_t)*m_instr) >> 20) & 0b11111;
    m_readData1 = Signal<32>(m_reg[readRegister1]);
    m_readData2 = Signal<32>(m_reg[readRegister2]);
}

void Registers::clock() {
    // if regWrite is high, write data to register
    if (*m_regWrite)
        m_reg[(uint32_t)*m_writeRegister] = (uint32_t)*m_writeData;
}

void Registers::setInputs(Signal<32>* instr, Signal<5>* writeReg, Signal<32>* writeData, Signal<1>* regWrite) {
    m_regWrite = regWrite;
    m_writeData = writeData;
    m_writeRegister = writeReg;
    m_instr = instr;
}

void Registers::init() {
    m_reg[0] = 0;
    m_reg[2] = STACKSTART;
    m_reg[3] = DATASTART;
}
