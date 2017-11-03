#include "runnercache.h"

#include "dmcache.h"
#include "facache.h"

RunnerCache::RunnerCache() {}

uint32_t RunnerCache::readData(uint32_t address) { return 0; }

void RunnerCache::writeData(uint32_t address) {}

void RunnerCache::createCacheLevel(cacheLevel level,
                                   CacheProperties properties) {
  // Creates a cache at the requested cache level with the given properties,
  // and sets the newly created cache as a childCache of a potential parent
  switch (level) {
  case L1:
    m_L1 = createCache(properties);
    break;
  case L2:
    m_L2 = createCache(properties);
    m_L1->setChildCache(m_L2.get());
    break;
  case L3:
    m_L3 = createCache(properties);
    m_L2->setChildCache(m_L3.get());
    break;
  }
}

std::unique_ptr<CacheBase>
RunnerCache::createCache(CacheProperties properties) {
  switch (properties.type) {
  case DM:
    return std::make_unique<DMCache>(properties);
  case SA:
  // return std::make_unique<>(properties);
  case FA:
    return std::make_unique<FACache>(properties);
  }
}

void RunnerCache::deleteCache(cacheLevel level) {
  switch (level) {
  case L1:
    m_L1.reset();
    break;
  case L2:
    m_L2.reset();
    break;
  case L3:
    m_L3.reset();
    break;
  }
}
