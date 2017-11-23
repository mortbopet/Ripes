#include "facache.h"

FACache::FACache(CacheProperties properties, int* cycleCounterPtr) : CacheBase(properties, cycleCounterPtr) {}

uint32_t FACache::readData(uint32_t /*address*/) {
    m_properties.readCount++;

    return 0;
}

void FACache::writeData(uint32_t /*address*/) {}

void FACache::resize(int /*size*/) {}

void FACache::searchCache(uint32_t /*address*/) const {}
