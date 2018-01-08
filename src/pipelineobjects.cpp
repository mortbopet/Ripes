#include "pipelineobjects.h"

// Instantiate register vector
std::vector<RegBase*> RegBase::registers;

void Registers::update() {
    // Read
    m_readData1 = Signal<32>(m_reg[(uint32_t)*m_readRegister1]);
    m_readData2 = Signal<32>(m_reg[(uint32_t)*m_readRegister2]);
}

void Registers::clock() {
    // if regWrite is high, write data to register (x0 is read-only)
    if (*m_regWrite && ((uint32_t)*m_writeRegister != 0))
        m_reg[(uint32_t)*m_writeRegister] = (uint32_t)*m_writeData;
    update();
}

void Registers::setInputs(Signal<5>* readRegister1, Signal<5>* readRegister2, Signal<5>* writeReg,
                          Signal<32>* writeData, Signal<1>* regWrite) {
    m_regWrite = regWrite;
    m_writeData = writeData;
    m_writeRegister = writeReg;
    m_readRegister1 = readRegister1;
    m_readRegister2 = readRegister2;
}

void Registers::init() {
    m_reg[0] = 0;
    m_reg[2] = STACKSTART;
    m_reg[3] = DATASTART;
}
