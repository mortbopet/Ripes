#include "cachebase.h"
#include "binutils.h"

#include <random>

namespace Ripes {

void CacheBase::updateCacheLineLRU(CacheLine& line, unsigned lruIdx) {
    // Find previous LRU value for the updated index
    const unsigned preLRU = line[lruIdx].lru;

    // All indicies which are curently more recent than preLRU shall be incremented
    for (auto& set : line) {
        if (set.second.valid && set.second.lru < preLRU) {
            set.second.lru++;
        }
    }

    // Upgrade @p lruIdx to the most recently used
    line[lruIdx].lru = 0;
}

void CacheBase::updateCacheValue(uint32_t address) {
    const unsigned lineIdx = getAccessLineIdx();
    const unsigned blockIdx = getAccessBlockIdx();

    auto& cacheLine = m_cacheLines[lineIdx];

    // Update based on replacement policy
    if (m_policy == CacheReplPlcy::Random) {
        // Select a random way
        m_currentWayIdx = std::rand() % getWays();
        auto& way = cacheLine[m_currentWayIdx];
        // Todo: this is not valid; the entire cache line (all blocks) should be read
        way.valid = true;
        way.tag = getAccessTag();
    } else if (m_policy == CacheReplPlcy::LRU) {
        if (getWays() == 1) {
            // Nothing to do if we are in LRU and only have 1 set
            m_currentWayIdx = 0;
            cacheLine[m_currentWayIdx].valid = true;
            cacheLine[m_currentWayIdx].tag = getAccessTag();
            return;
        }

        // Ensure that all ways in the cache line has been initialized
        for (int i = 0; i < getWays(); i++) {
            cacheLine[i];
        }

        // If there is an invalid cache line, select that
        unsigned wayIdx = 0;
        CacheWay* selectedWay = nullptr;
        for (auto& way : cacheLine) {
            if (!way.second.valid) {
                wayIdx = way.first;
                selectedWay = &way.second;
                break;
            }
        }
        if (selectedWay == nullptr) {
            // Else, Find LRU way
            for (auto& way : cacheLine) {
                if (way.second.lru == getWays() - 1) {
                    wayIdx = way.first;
                    selectedWay = &way.second;
                }
            }
        }

        Q_ASSERT(selectedWay != nullptr && "There must have been an issue with setting the LRU bits");

        m_currentWayIdx = wayIdx;
        selectedWay->valid = true;
        selectedWay->tag = getAccessTag();
        updateCacheLineLRU(cacheLine, wayIdx);
    }
}

void CacheBase::updateHitRate() {
    if (m_accessTrace.size() == 0) {
        m_hitrate = 0;
    } else {
        auto& trace = m_accessTrace.rbegin()->second;
        m_hitrate = static_cast<double>(trace.hits) / (trace.hits + trace.misses);
    }
}

void CacheBase::analyzeCacheAccess() {
    const unsigned lineIdx = getAccessLineIdx();

    m_currentAccessLine = &m_cacheLines[lineIdx];

    m_currentAccessIsHit = false;
    if (m_cacheLines.count(lineIdx) != 0) {
        int wayIdx = 0;
        for (const auto& way : m_cacheLines.at(lineIdx)) {
            if (way.second.tag == getAccessTag() && way.second.valid) {
                m_currentWayIdx = wayIdx;
                m_currentAccessIsHit = true;
                break;
            }
            wayIdx++;
        }
    }

    // Update cache access trace for the current cycle
    if (m_accessTrace.size() == 0) {
        m_accessTrace[0] = CacheAccessTrace(m_currentAccessIsHit);
    } else {
        m_accessTrace[m_accessTrace.size()] =
            CacheAccessTrace(m_accessTrace[m_accessTrace.size() - 1], m_currentAccessIsHit);
    }
    updateHitRate();
    emit hitRateChanged(m_hitrate);

    return;
}

void CacheBase::read(uint32_t address) {
    m_currentAccessAddress = address;
    analyzeCacheAccess();
    if (!m_currentAccessIsHit) {
        updateCacheValue(m_currentAccessAddress);
    }
    emit dataChanged(m_currentAccessAddress);
}
void CacheBase::write(uint32_t address) {
    m_currentAccessAddress = address;
    emit accessChanged(m_currentAccessAddress);
}
void CacheBase::undo() {}

unsigned CacheBase::getAccessLineIdx() const {
    uint32_t maskedAddress = m_currentAccessAddress & m_lineMask;
    maskedAddress >>= 2 + getBlockBits();
    return maskedAddress;
}

unsigned CacheBase::getAccessWayIdx() const {
    return m_currentWayIdx;
}

unsigned CacheBase::getAccessTag() const {
    uint32_t maskedAddress = m_currentAccessAddress & m_tagMask;
    maskedAddress >>= 2 + getBlockBits() + getLineBits();
    return maskedAddress;
}

unsigned CacheBase::getAccessBlockIdx() const {
    uint32_t maskedAddress = m_currentAccessAddress & m_blockMask;
    maskedAddress >>= 2;
    return maskedAddress;
}

void CacheBase::updateConfiguration() {
    // Cache configuration changes shall enforce a full reset of the computing system
    m_cacheLines.clear();
    m_accessTrace.clear();

    // Recalculate masks
    int bitoffset = 2;  // 2^2 = 4-byte offset (32-bit words in cache)

    m_blockMask = generateBitmask(getBlockBits()) << bitoffset;
    bitoffset += getBlockBits();

    m_lineMask = generateBitmask(getLineBits()) << bitoffset;
    bitoffset += getLineBits();

    m_tagMask = generateBitmask(32 - bitoffset) << bitoffset;

    emit hitRateChanged(m_hitrate);
    emit configurationChanged();
}

}  // namespace Ripes
