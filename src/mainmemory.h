#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#include "defines.h"

class MainMemory {
public:
    explicit MainMemory();

    void write(uint32_t address, uint32_t value, int size);
    uint32_t read(uint32_t addess);
    void clear() { m_memory.clear(); }

private:
    memory m_memory;
};

#endif  // MAiNMEMORY_H
