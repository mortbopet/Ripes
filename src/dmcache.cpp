#include "dmcache.h"

DMCache::DMCache(CacheProperties properties, int* cycleCounterPtr) : CacheBase(properties, cycleCounterPtr) {
    assertSize(m_properties.size);

    // Initialize cache to all zeros and invalidated
    for (auto i = 0; i < m_properties.size; i++) {
        // Initialize cache line to 0, and invalid
        m_cache[i] = std::pair<bool, uint32_t>(false, 0);
    }
}

uint32_t DMCache::readData(uint32_t address) {
    m_properties.readCount++;

    auto it = m_cache.find(address);
    if (it != m_cache.end() && it->second.first) {
        // Cache hit
        return it->second.second;
    } else {
        // Cache miss
        m_properties.missCount++;
        *m_cycleCounterPtr += m_properties.missPenalty;
        if (m_childCache) {
            // Cache has subcache, delegate memory request to subcache
            return m_childCache->readData(address);
        } else {
            // No subcaches available, read straight from memory
            *m_cycleCounterPtr += m_properties.memoryPenalty;
            // @TODO : return an actual memory value here!
            return 0;
        }
    }
}

void DMCache::resize(int /*size*/) {}

void DMCache::writeData(uint32_t /*address*/) {}

void DMCache::searchCache(uint32_t /*address*/) const {}

void CacheBase::resize(int size) {
    assertSize(size);
    // do resize...
}
