#include "cachesim.h"
#include "binutils.h"

#include <random>

namespace Ripes {

CacheSim::CacheSim(QObject* parent) : QObject(parent) {
    updateConfiguration();
}

void CacheSim::updateCacheLineLRU(CacheLine& line, unsigned lruIdx) {
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

CacheSim::CacheSize CacheSim::getCacheSize() const {
    CacheSize size;

    const int entries = getLines() * getWays();

    // Valid bits
    unsigned componentBits = entries;  // 1 bit per entry
    size.components.push_back("Valid bits: " + QString::number(componentBits));
    size.bits += componentBits;

    if (m_wrPolicy == WritePolicy::WriteBack) {
        // Dirty bits
        unsigned componentBits = entries;  // 1 bit per entry
        size.components.push_back("Dirty bits: " + QString::number(componentBits));
        size.bits += componentBits;
    }

    if (m_replPolicy == ReplPolicy::LRU) {
        // LRU bits
        componentBits = getWaysBits() * entries;
        size.components.push_back("LRU bits: " + QString::number(componentBits));
        size.bits += componentBits;
    }

    // Tag bits
    componentBits = bitcount(m_tagMask) * entries;
    size.components.push_back("Tag bits: " + QString::number(componentBits));
    size.bits += componentBits;

    // Data bits
    componentBits = 32 * entries * getBlocks();
    size.components.push_back("Data bits: " + QString::number(componentBits));
    size.bits += componentBits;

    return size;
}

void CacheSim::evictAndUpdate(CacheTransaction& transaction) {
    auto& cacheLine = m_cacheLines[transaction.lineIdx];

    CacheWay* currentWay = nullptr;

    // Update based on replacement policy
    if (m_replPolicy == ReplPolicy::Random) {
        // Select a random way
        transaction.wayIdx = std::rand() % getWays();
        currentWay = &cacheLine[transaction.wayIdx];
    } else if (m_replPolicy == ReplPolicy::LRU) {
        if (getWays() == 1) {
            // Nothing to do if we are in LRU and only have 1 set
            transaction.wayIdx = 0;
            currentWay = &cacheLine[transaction.wayIdx];
        } else {
            // Lazily all ways in the cacheline before starting to iterate
            for (int i = 0; i < getWays(); i++) {
                cacheLine[i];
            }

            // If there is an invalid cache line, select that
            for (auto& way : cacheLine) {
                if (!way.second.valid) {
                    transaction.wayIdx = way.first;
                    currentWay = &way.second;
                    break;
                }
            }
            if (currentWay == nullptr) {
                // Else, Find LRU way
                for (auto& way : cacheLine) {
                    if (way.second.lru == getWays() - 1) {
                        transaction.wayIdx = way.first;
                        currentWay = &way.second;
                    }
                }
            }
            Q_ASSERT(transaction.wayIdx != s_invalidIndex);
            Q_ASSERT(currentWay != nullptr && "There must have been an issue with setting the LRU bits");
        }
    }

    if (!currentWay->valid) {
        // Record that this was an invalid->valid transition
        transaction.transToValid = true;
        currentWay->valid = true;
    }

    currentWay->dirty = false;
    currentWay->tag = transaction.tag;
    transaction.tagChanged = true;
}

unsigned CacheSim::getHits() const {
    if (m_accessTrace.size() == 0) {
        return 0;
    } else {
        auto& trace = m_accessTrace.rbegin()->second;
        return trace.hits;
    }
}

unsigned CacheSim::getMisses() const {
    if (m_accessTrace.size() == 0) {
        return 0;
    } else {
        auto& trace = m_accessTrace.rbegin()->second;
        return trace.misses;
    }
}

double CacheSim::getHitRate() const {
    if (m_accessTrace.size() == 0) {
        return 0;
    } else {
        auto& trace = m_accessTrace.rbegin()->second;
        return static_cast<double>(trace.hits) / (trace.hits + trace.misses);
    }
}

void CacheSim::analyzeCacheAccess(CacheTransaction& transaction) {
    transaction.lineIdx = getLineIdx(transaction.address);
    transaction.blockIdx = getBlockIdx(transaction.address);
    transaction.tag = getTag(transaction.address);

    transaction.isHit = false;
    if (m_cacheLines.count(transaction.lineIdx) != 0) {
        for (const auto& way : m_cacheLines.at(transaction.lineIdx)) {
            if ((way.second.tag == transaction.tag) && way.second.valid) {
                transaction.wayIdx = way.first;
                transaction.isHit = true;
                break;
            }
        }
    }

    // Update cache access trace for the current cycle
    if (m_accessTrace.size() == 0) {
        m_accessTrace[0] = CacheAccessTrace(transaction.isHit);
    } else {
        m_accessTrace[m_accessTrace.size()] =
            CacheAccessTrace(m_accessTrace[m_accessTrace.size() - 1], transaction.isHit);
    }
    emit hitrateChanged();
}

void CacheSim::access(uint32_t address, AccessType type) {
    address = address & ~0b11;  // Disregard unaligned accesses
    CacheTransaction transaction;
    transaction.address = address;
    transaction.type = type;

    analyzeCacheAccess(transaction);

    if (!transaction.isHit) {
        if (type == AccessType::Read ||
            (type == AccessType::Write && getWriteAllocPolicy() == WriteAllocPolicy::WriteAllocate)) {
            evictAndUpdate(transaction);
        }
    }

    if (transaction.wayIdx != s_invalidIndex) {
        // Lazily ensure that the located way has been initialized
        m_cacheLines[transaction.lineIdx][transaction.wayIdx];

        if (type == AccessType::Write) {
            CacheWay& way = m_cacheLines[transaction.lineIdx][transaction.wayIdx];
            way.dirty = true;
        }

        if (getReplacementPolicy() == ReplPolicy::LRU) {
            updateCacheLineLRU(m_cacheLines[transaction.lineIdx], transaction.wayIdx);
        }
    }

    emit dataChanged(transaction);
}

void CacheSim::undo() {}

unsigned CacheSim::getLineIdx(const uint32_t address) const {
    uint32_t maskedAddress = address & m_lineMask;
    maskedAddress >>= 2 + getBlockBits();
    return maskedAddress;
}

unsigned CacheSim::getTag(const uint32_t address) const {
    uint32_t maskedAddress = address & m_tagMask;
    maskedAddress >>= 2 + getBlockBits() + getLineBits();
    return maskedAddress;
}

unsigned CacheSim::getBlockIdx(const uint32_t address) const {
    uint32_t maskedAddress = address & m_blockMask;
    maskedAddress >>= 2;
    return maskedAddress;
}

const CacheSim::CacheLine* CacheSim::getLine(unsigned idx) const {
    if (m_cacheLines.count(idx)) {
        return &m_cacheLines.at(idx);
    } else {
        return nullptr;
    }
}

void CacheSim::updateConfiguration() {
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

    emit configurationChanged();
}

void CacheSim::setBlocks(unsigned blocks) {
    m_blocks = blocks;
    updateConfiguration();
}
void CacheSim::setLines(unsigned lines) {
    m_lines = lines;
    updateConfiguration();
}
void CacheSim::setWays(unsigned ways) {
    m_ways = ways;
    updateConfiguration();
}

void CacheSim::setPreset(const CachePreset& preset) {
    m_blocks = preset.blocks;
    m_ways = preset.ways;
    m_lines = preset.lines;
    m_wrPolicy = preset.wrPolicy;
    m_wrAllocPolicy = preset.wrAllocPolicy;
    m_replPolicy = preset.replPolicy;

    updateConfiguration();
}

}  // namespace Ripes
