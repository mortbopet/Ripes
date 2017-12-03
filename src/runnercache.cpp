#include "runnercache.h"

#include "dmcache.h"
#include "facache.h"

#include "cachetab.h"

RunnerCache::RunnerCache() {}

uint32_t RunnerCache::readData(uint32_t /*address*/) {
    return 0;
}

void RunnerCache::writeData(uint32_t /*address*/) {}

void RunnerCache::setCacheLevel(cacheLevel level, bool enable, CacheProperties properties) {
    // Creates a cache at the requested cache level with the given properties,
    // and sets the newly created cache as a childCache of a potential parent
    if (enable) {
        switch (level) {
            case cacheLevel::L1:
                m_L1 = createCache(properties);
                m_cacheTabPtr->connectSetupWidget(m_L1.get(), cacheLevel::L1);
                break;
            case cacheLevel::L2:
                m_L2 = createCache(properties);
                m_L1->setChildCache(m_L2.get());
                m_cacheTabPtr->connectSetupWidget(m_L2.get(), cacheLevel::L2);
                break;
            case cacheLevel::L3:
                m_L3 = createCache(properties);
                m_L2->setChildCache(m_L3.get());
                m_cacheTabPtr->connectSetupWidget(m_L3.get(), cacheLevel::L3);
                break;
        }
    } else {
        switch (level) {
            case cacheLevel::L1:
                m_L1.reset();
                break;
            case cacheLevel::L2:
                m_L2.reset();
                break;
            case cacheLevel::L3:
                m_L3.reset();
                break;
        }
    }
}

std::unique_ptr<CacheBase> RunnerCache::createCache(CacheProperties properties) {
    switch (properties.type) {
        case cacheType::DM:
            return std::make_unique<DMCache>(properties);
        case cacheType::SA:
        // return std::make_unique<>(properties);
        case cacheType::FA:
            return std::make_unique<FACache>(properties);
        default:
            return std::make_unique<DMCache>(properties);
    }
}
