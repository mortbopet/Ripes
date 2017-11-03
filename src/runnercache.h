#ifndef RUNNERCACHE_H
#define RUNNERCACHE_H

#include "cachebase.h"
#include <memory>

// Class for handling cache fetching and structuring, for the runner

class RunnerCache {
public:
  RunnerCache();

public:
  void createCacheLevel(cacheLevel level, CacheProperties properties);
  void deleteCache(cacheLevel level);
  void modifyCache(cacheLevel level, CacheProperties properties);

  uint32_t readData(uint32_t address);
  void writeData(uint32_t address);

private:
  std::unique_ptr<CacheBase> createCache(CacheProperties properties);
  std::unique_ptr<CacheBase> m_L1;
  std::unique_ptr<CacheBase> m_L2;
  std::unique_ptr<CacheBase> m_L3;
};

#endif // RUNNERCACHE_H
