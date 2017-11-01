#include "dmcache.h"

DMCache::DMCache(int size) : Cache(size) {}

uint32_t DMCache::readData(uint32_t address) const { return 0; }

void DMCache::writeData(uint32_t address) {}

void DMCache::searchCache(uint32_t address) const {}
