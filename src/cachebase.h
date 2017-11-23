#ifndef CACHE_H
#define CACHE_H

#include "defines.h"

#include <cstdint>
#include <vector>

typedef struct {
    int readCount = 0;
    int missCount = 0;

    int missPenalty;
    int searchPenalty;
    int memoryPenalty;
    cacheType type;

    int size;
} CacheProperties;

// Base class for all cache types

class CacheBase {
public:
    CacheBase(CacheProperties properties, int* cycleCounterPtr);
    virtual uint32_t readData(uint32_t address) = 0;
    virtual void writeData(uint32_t address) = 0;

    void setMissPenalty(int cycles) { m_properties.missPenalty = cycles; }
    void setSearchPenalty(int cycles) { m_properties.searchPenalty = cycles; }
    void setChildCache(CacheBase* childCache) { m_childCache = childCache; }
    virtual void resize(int size) = 0;

protected:
    void assertSize(int size) const;

    CacheProperties m_properties;

    uint32_t* m_memoryPtr;
    int* m_cycleCounterPtr;             // pointer to Runner's cycle counter
    CacheBase* m_childCache = nullptr;  // pointer to child cache (ie. L1 -> L2)
};

#endif  // CACHE_H
