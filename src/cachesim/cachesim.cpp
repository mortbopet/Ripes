#include "cachesim.h"
#include "binutils.h"

#include <random>
#include <utility>

namespace Ripes {

CacheSim::CacheSim(QObject* parent) : QObject(parent) {
    updateConfiguration();
}

void CacheSim::updateCacheLineReplFields(CacheLine& line, unsigned wayIdx) {
    if (getReplacementPolicy() == ReplPolicy::LRU) {
        // Find previous LRU value for the updated index
        const unsigned preLRU = line[wayIdx].lru;

        // All indicies which are curently more recent than preLRU shall be incremented
        for (auto& set : line) {
            if (set.second.valid && set.second.lru < preLRU) {
                set.second.lru++;
            }
        }

        // Upgrade @p lruIdx to the most recently used
        line[wayIdx].lru = 0;
    }
}

void CacheSim::revertCacheLineReplFields(CacheLine& line, const CacheWay& oldWay, unsigned wayIdx) {
    if (getReplacementPolicy() == ReplPolicy::LRU) {
        // All indicies which are curently less than or equal to the old LRU shall be decremented
        for (auto& set : line) {
            if (set.second.valid && set.second.lru <= oldWay.lru) {
                set.second.lru--;
            }
        }

        // Revert the oldWay LRU
        line[wayIdx].lru = oldWay.lru;
    }
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

std::pair<unsigned, CacheSim::CacheWay*> CacheSim::locateEvictionWay(const CacheTransaction& transaction) {
    auto& cacheLine = m_cacheLines[transaction.index.line];

    std::pair<unsigned, CacheSim::CacheWay*> ew;
    ew.first = s_invalidIndex;
    ew.second = nullptr;

    // Locate a new way based on replacement policy
    if (m_replPolicy == ReplPolicy::Random) {
        // Select a random way
        ew.first = std::rand() % getWays();
        ew.second = &cacheLine[ew.first];
    } else if (m_replPolicy == ReplPolicy::LRU) {
        if (getWays() == 1) {
            // Nothing to do if we are in LRU and only have 1 set
            ew.first = 0;
            ew.second = &cacheLine[ew.first];
        } else {
            // Lazily all ways in the cacheline before starting to iterate
            for (int i = 0; i < getWays(); i++) {
                cacheLine[i];
            }

            // If there is an invalid cache line, select that
            for (auto& way : cacheLine) {
                if (!way.second.valid) {
                    ew.first = way.first;
                    ew.second = &way.second;
                    break;
                }
            }
            if (ew.second == nullptr) {
                // Else, Find LRU way
                for (auto& way : cacheLine) {
                    if (way.second.lru == getWays() - 1) {
                        ew.first = way.first;
                        ew.second = &way.second;
                        break;
                    }
                }
            }
        }
    }

    Q_ASSERT(ew.first != s_invalidIndex && "Unable to locate way for eviction");
    Q_ASSERT(ew.second != nullptr && "Unable to locate way for eviction");
    return ew;
}

CacheSim::CacheWay CacheSim::evictAndUpdate(CacheTransaction& transaction) {
    const auto [wayIdx, wayPtr] = locateEvictionWay(transaction);

    CacheWay eviction;

    if (!wayPtr->valid) {
        // Record that this was an invalid->valid transition
        transaction.transToValid = true;
    } else {
        // Store the old way info in our eviction trace, in case of rollbacks
        eviction = *wayPtr;
    }

    // Invalidate the target way
    *wayPtr = CacheWay();

    // Set required values in way, reflecting the newly loaded address
    wayPtr->valid = true;
    wayPtr->dirty = false;
    wayPtr->tag = getTag(transaction.address);
    transaction.tagChanged = true;
    transaction.index.way = wayIdx;

    return eviction;
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

void CacheSim::analyzeCacheAccess(CacheTransaction& transaction) const {
    transaction.index.line = getLineIdx(transaction.address);
    transaction.index.block = getBlockIdx(transaction.address);

    transaction.isHit = false;
    if (m_cacheLines.count(transaction.index.line) != 0) {
        for (const auto& way : m_cacheLines.at(transaction.index.line)) {
            if ((way.second.tag == getTag(transaction.address)) && way.second.valid) {
                transaction.index.way = way.first;
                transaction.isHit = true;
                break;
            }
        }
    }
}

void CacheSim::updateHitTrace(const CacheTransaction& transaction) {
    if (m_accessTrace.size() == 0) {
        m_accessTrace[0] = CacheAccessTrace(transaction.isHit);
    } else {
        m_accessTrace[m_accessTrace.size()] =
            CacheAccessTrace(m_accessTrace[m_accessTrace.size() - 1], transaction.type, transaction.isHit, false);
    }
    emit hitrateChanged();
}

void CacheSim::access(uint32_t address, AccessType type) {
    address = address & ~0b11;  // Disregard unaligned accesses
    CacheTrace trace;
    CacheWay oldWay;
    CacheTransaction transaction;
    transaction.address = address;
    transaction.type = type;

    analyzeCacheAccess(transaction);
    updateHitTrace(transaction);

    if (!transaction.isHit) {
        if (type == AccessType::Read ||
            (type == AccessType::Write && getWriteAllocPolicy() == WriteAllocPolicy::WriteAllocate)) {
            oldWay = evictAndUpdate(transaction);
        }
    } else {
        oldWay = m_cacheLines[transaction.index.line][transaction.index.way];
    }

    // At this point, no further changes shall be made to the transaction.
    // We record the transaction as well as a possible eviction
    trace.oldWay = oldWay;
    trace.transaction = transaction;
    pushTrace(trace);

    // Update dirty and LRU bits.
    // Initially, we need a check for the case of "write + miss + noWriteAlloc". In this case, we should not update
    // replacement/dirty fields. In all other cases, this is a valid action.
    const bool writeMissNoAlloc =
        !transaction.isHit && type == AccessType::Write && getWriteAllocPolicy() == WriteAllocPolicy::NoWriteAllocate;

    if (!writeMissNoAlloc) {
        // Lazily ensure that the located way has been initialized
        m_cacheLines[transaction.index.line][transaction.index.way];

        if (type == AccessType::Write && getWritePolicy() == WritePolicy::WriteBack) {
            CacheWay& way = m_cacheLines[transaction.index.line][transaction.index.way];
            way.dirty = true;
            way.dirtyBlocks.insert(transaction.index.block);
        }

        updateCacheLineReplFields(m_cacheLines[transaction.index.line], transaction.index.way);
    }

    // === Some sanity checking ===
    // It should never be possible that a read returns an invalid way index
    if (type == AccessType::Read) {
        transaction.index.assertValid();
    }

    // It should never be possible that a write returns an invalid way index if we write-allocate
    if (type == AccessType::Write && getWriteAllocPolicy() == WriteAllocPolicy::WriteAllocate) {
        transaction.index.assertValid();
    }
    // ===========================

    if (writeMissNoAlloc) {
        // There are no graphical changes to perform since nothing is pulled into the cache upon a missed write without
        // write allocation
        return;
    }

    emit dataChanged(transaction);
}

void CacheSim::undo() {
    if (m_traceStack.size() == 0)
        return;

    const auto trace = popTrace();

    const auto& oldWay = trace.oldWay;
    const auto& transaction = trace.transaction;
    const unsigned& lineIdx = trace.transaction.index.line;
    const unsigned& blockIdx = trace.transaction.index.block;
    const unsigned& wayIdx = trace.transaction.index.way;
    auto& line = m_cacheLines.at(lineIdx);
    auto& way = line.at(wayIdx);

    // Case 1: A cache way was transitioned to valid. In this case, we simply invalidate the cache way
    if (trace.transaction.transToValid) {
        // Invalidate the way
        Q_ASSERT(m_cacheLines.at(lineIdx).count(wayIdx) != 0);
        way = CacheWay();
    }
    // Case 2: A miss occured on a valid entry. In this case, we have to restore the old way, which was evicted
    // - Restore the old entry which was evicted
    else if (!trace.transaction.isHit) {
        way = oldWay;
    }
    // Case 3: Else, it was a cache hit, and only the replacement fields needs to be updated

    revertCacheLineReplFields(line, oldWay, wayIdx);

    // Notify that changes to teh way has been performed
    emit wayInvalidated(lineIdx, wayIdx);
}

CacheSim::CacheTrace CacheSim::popTrace() {
    Q_ASSERT(m_traceStack.size() > 0);
    auto val = m_traceStack.front();
    m_traceStack.pop_front();
    return val;
}

void CacheSim::pushTrace(const CacheTrace& eviction) {
    m_traceStack.push_front(eviction);
    if (m_traceStack.size() > vsrtl::core::ClockedComponent::reverseStackSize()) {
        m_traceStack.pop_back();
    }
}

uint32_t CacheSim::buildAddress(unsigned tag, unsigned lineIdx, unsigned blockIdx) const {
    uint32_t address = 0;
    address |= tag << (2 /*byte offset*/ + getBlockBits() + getLineBits());
    address |= lineIdx << (2 /*byte offset*/ + getBlockBits());
    address |= blockIdx << (2 /*byte offset*/);
    return address;
}

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
    m_traceStack.clear();

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
