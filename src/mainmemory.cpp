#include "mainmemory.h"

MainMemory::MainMemory() {}

void MainMemory::write(uint32_t address, uint32_t value, int size) {
    // writes value to from the given address start, and up to $size bytes of
    // $value
    // Using the hashtable, new allocations will automatically be handled
    for (int i = 0; i < size; i++) {
        m_memory[address + i] = value & 0xff;
        value >>= 8;
    }
}

uint32_t MainMemory::read(uint32_t address) {
    // Note: If address is not found in memory map, a default constructed object
    // will be created, and read. in our case uint8_t() = 0
    uint32_t read = (m_memory[address] | (m_memory[address + 1] << 8) | (m_memory[address + 2] << 16) |
                     (m_memory[address + 3] << 24));
    return read;
}
