#include "dmcache.h"

DMCache::DMCache(int size) : Cache() {
  assertSize(size);

  // Initialize cache to all zeros and invalidated
  for (auto i = 0; i < size; i++) {
    // Initialize cache line to 0, and invalid
    m_cache[i] = std::pair<bool, uint32_t>(false, 0);
  }
}

uint32_t DMCache::readData(uint32_t address) {
  m_readCount++;

  auto it = m_cache.find(address);
  if (it != m_cache.end() && it->second.first) {
    // Cache hit
    return it->second.second;
  } else {
    // Cache miss
    m_missCount++;
  }
}

void DMCache::writeData(uint32_t address) {}

void DMCache::searchCache(uint32_t address) const {}

void Cache::resize(int size) {
  assertSize(size);
  // do resize...
}
