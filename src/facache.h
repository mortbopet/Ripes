#ifndef FACACHE_H
#define FACACHE_H

#include "cachebase.h"

// Fully associative cache

class FACache : public CacheBase {
public:
    FACache(CacheProperties properties, int* cycleCounterPtr = nullptr);

    uint32_t readData(uint32_t address) override;
    void writeData(uint32_t address) override;
    void resize(int size) override;

private:
    void searchCache(uint32_t address) const;
};

#endif  // FACACHE_H
