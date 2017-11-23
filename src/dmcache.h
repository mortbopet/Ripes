#ifndef DMCACHE_H
#define DMCACHE_H

#include "cachebase.h"
#include <cstdint>

#include <map>

// Direct mapped cache
class DMCache : public CacheBase {
public:
    DMCache(CacheProperties properties, int* cycleCounterPtr = nullptr);

    uint32_t readData(uint32_t address) override;
    void writeData(uint32_t address) override;
    void resize(int size) override;

private:
    void searchCache(uint32_t address) const;

    // Cache is a map of an (up to) 32-bit tag, and a pair of a valid-bit and the
    // actual data
    std::map<uint32_t, std::pair<bool, uint32_t>> m_cache;
};

#endif  // DMCACHE_H
