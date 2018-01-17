#ifndef RUNNERCACHE_H
#define RUNNERCACHE_H

#include <memory>
#include "cachebase.h"
class CacheTab;

// Class for handling cache fetching and structuring, for the runner
namespace {
CacheProperties defaultProperties;
}

class RunnerCache {
    friend class CacheTab;

public:
    RunnerCache();

public:
    void setCacheLevel(cacheLevel level, bool enable, CacheProperties properties = defaultProperties);
    void deleteCache(cacheLevel level);
    void modifyCache(cacheLevel level, CacheProperties properties);

    uint32_t readData(uint32_t address);
    void writeData(uint32_t address);

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool state) { m_enabled = state; }

private:
    std::unique_ptr<CacheBase> createCache(CacheProperties properties);
    std::unique_ptr<CacheBase> m_L1;
    std::unique_ptr<CacheBase> m_L2;
    std::unique_ptr<CacheBase> m_L3;

    bool m_enabled = false;
    CacheTab* m_cacheTabPtr;
};

#endif  // RUNNERCACHE_H
