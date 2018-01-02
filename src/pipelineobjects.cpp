#include "pipelineobjects.h"

void Registers::update() {
    // Read
    int readRegister1 = (((uint32_t)*m_instr) >> 15) & 0b11111;
    int readRegister2 = (((uint32_t)*m_instr) >> 20) & 0b11111;
    m_readData1 = Signal<32>(m_reg[readRegister1]);
    m_readData2 = Signal<32>(m_reg[readRegister2]);

    // Write
    if (*m_regWrite)
        m_reg[(uint32_t)*m_writeRegister] = (uint32_t)*m_writeData;
}

void Registers::setInputs(Signal<32>* instr, Signal<5>* writeReg, Signal<32>* writeData) {
    m_regWrite = nullptr;
    m_writeData = writeData;
    m_writeRegister = writeReg;
    m_instr = instr;
}
