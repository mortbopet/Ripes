#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#include "defines.h"

// RVAccess struct - used for keeping track of read and write access to memory, for displaying in GUI
enum class RW { Read, Write };
typedef struct {
    uint32_t pc;  // GUI converts pc value to an instruction
    RW rw;
    uint32_t addr;
    uint32_t cycle;
} RVAccess;

template <typename key_T, typename value_T>
class MainMemoryTemplate : public std::unordered_map<key_T, value_T> {
public:
    void write(uint32_t address, uint32_t value, int size) {
        // writes value to from the given address start, and up to $size bytes of
        // $value
        // Using the hashtable, new allocations will automatically be handled
        for (int i = 0; i < size; i++) {
            (*this)[address + i] = value & 0xff;
            value >>= 8;
        }
    }
    uint32_t read(uint32_t address) {
        // Note: If address is not found in memory map, a default constructed object
        // will be created, and read. in our case uint8_t() = 0
        uint32_t read = ((*this)[address] | ((*this)[address + 1] << 8) | ((*this)[address + 2] << 16) |
                         ((*this)[address + 3] << 24));
        return read;
    }
    void reset(uint32_t textSize) {
        // Reset memory - erase all elements which is not part of the textsegment of the program
        for (auto it = this->begin(); it != this->end();) {
            // Use standard associative-container erasing. If erased, set iterator to iterator after the erased value
            // (default return value from .erase), else, increment iterator;
            if (it->first > textSize) {
                it = this->erase(it);
            } else {
                ++it;
            }
        }
    }
};

typedef MainMemoryTemplate<uint32_t, uint8_t> MainMemory;

#endif  // MAiNMEMORY_H
