#include "facache.h"

FACache::FACache(int size, int *cycleCounterPtr) : Cache(cycleCounterPtr) {}

uint32_t FACache::readData(uint32_t address) {
  m_readCount++;

  return 0;
}

void FACache::writeData(uint32_t address) {}

void FACache::searchCache(uint32_t address) const {}
