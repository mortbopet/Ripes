#include "cachesim.h"
#include "binutils.h"

#include "processorhandler.h"

#include <QApplication>
#include <QThread>
#include <random>
#include <utility>

namespace Ripes {

CacheSim::CacheSim(QObject* parent) : QObject(parent) {
    connect(ProcessorHandler::get(), &ProcessorHandler::reqProcessorReset, this, &CacheSim::processorReset);

    connect(ProcessorHandler::get(), &ProcessorHandler::runFinished, this, [=] {
        // Given that we are not updating the graphical state of the cache simulator whilst the processor is running,
        // once running is finished, the entirety of the cache view should be reloaded in the graphical view.
        emit hitrateChanged();
        emit cacheInvalidated();
    });

    updateConfiguration();
}

void CacheSim::setType(CacheSim::CacheType type) {
    m_type = type;
    reassociateMemory();
}

void CacheSim::reassociateMemory() {
    if (m_type == CacheType::DataCache) {
        m_memory.rw = ProcessorHandler::get()->getDataMemory();
    } else if (m_type == CacheType::InstrCache) {
        m_memory.rom = ProcessorHandler::get()->getInstrMemory();
    } else {
        Q_ASSERT(false);
    }
    Q_ASSERT(m_memory.rw != nullptr);
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

        if (eviction.dirty) {
            // The eviction will result in a writeback
            transaction.isWriteback = true;
        }
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

unsigned CacheSim::getWritebacks() const {
    if (m_accessTrace.size() == 0) {
        return 0;
    } else {
        auto& trace = m_accessTrace.rbegin()->second;
        return trace.writebacks;
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

void CacheSim::pushAccessTrace(const CacheTransaction& transaction) {
    // Access traces are pushed in sorted order into the access trace map; indexed by a key corresponding to the cycle
    // of the acces.
    const unsigned currentCycle = ProcessorHandler::get()->getProcessor()->getCycleCount();

    const CacheAccessTrace& mostRecentTrace =
        m_accessTrace.size() == 0 ? CacheAccessTrace() : m_accessTrace.rbegin()->second;

    m_accessTrace[currentCycle] = CacheAccessTrace(mostRecentTrace, transaction);

    if (!isAsynchronouslyAccessed()) {
        emit hitrateChanged();
    }
}

void CacheSim::popAccessTrace() {
    Q_ASSERT(m_accessTrace.size() > 0);
    // The access trace should have an entry
    m_accessTrace.erase(m_accessTrace.rbegin()->first);
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

    if (!transaction.isHit) {
        if (type == AccessType::Read ||
            (type == AccessType::Write && getWriteAllocPolicy() == WriteAllocPolicy::WriteAllocate)) {
            oldWay = evictAndUpdate(transaction);
        }
    } else {
        oldWay = m_cacheLines[transaction.index.line][transaction.index.way];
    }

    // === Update dirty and LRU bits ===

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
    } else {
        // In case of a write miss with no write allocate, the value is always written through to memory (a writeback)
        transaction.isWriteback = true;
    }

    // If our WritePolicy is WriteThrough and this access is a write, the transaction will always result in a WriteBack
    if (type == AccessType::Write && getWritePolicy() == WritePolicy::WriteThrough) {
        transaction.isWriteback = true;
    }

    // ===========================

    // At this point, no further changes shall be made to the transaction.
    // We record the transaction as well as a possible eviction
    trace.oldWay = oldWay;
    trace.transaction = transaction;
    pushTrace(trace);
    pushAccessTrace(transaction);

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

    if (isAsynchronouslyAccessed()) {
        return;
    }

    emit dataChanged(&transaction);
}

bool CacheSim::isAsynchronouslyAccessed() const {
    return QThread::currentThread() != QApplication::instance()->thread();
}

void CacheSim::undo() {
    if (m_traceStack.size() == 0)
        return;

    const auto trace = popTrace();
    popAccessTrace();

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
    // Case 3: Else, it was a cache hit; Revert replacement fields and dirty blocks
    way.dirtyBlocks = oldWay.dirtyBlocks;
    revertCacheLineReplFields(line, oldWay, wayIdx);

    // Notify that changes to the way has been performed
    emit wayInvalidated(lineIdx, wayIdx);

    // Finally, re-emit the transaction which occurred in the previous cache access to update the cache
    // highlighting state
    if (m_traceStack.size() > 0) {
        emit dataChanged(&m_traceStack.begin()->transaction);
    } else {
        emit dataChanged(nullptr);
    }
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

void CacheSim::processorWasClocked() {
    if (m_type == CacheType::DataCache) {
        AccessType type;
        // Determine whether the memory is being accessed in the current cycle, and if so, the access type.
        switch (m_memory.rw->op.uValue()) {
            case MemOp::SB:
            case MemOp::SH:
            case MemOp::SW:
                if (m_memory.rw->wr_en.uValue() == 1) {
                    type = AccessType::Write;
                    break;
                } else {
                    // Nothing to do
                    return;
                }
            case MemOp::LB:
            case MemOp::LBU:
            case MemOp::LH:
            case MemOp::LHU:
            case MemOp::LW:
                type = AccessType::Read;
                break;
            case MemOp::NOP:
            default:
                // Nothing to do
                return;
        }

        access(m_memory.rw->addr.uValue(), type);
    } else {
        // ROM; read in every cycle
        access(m_memory.rom->addr.uValue(), AccessType::Read);
    }
}

void CacheSim::processorWasReversed() {
    if (m_accessTrace.size() == 0) {
        // Nothing to reverse
        return;
    }

    const unsigned cycleToUndo = ProcessorHandler::get()->getProcessor()->getCycleCount() + 1;
    if (m_accessTrace.rbegin()->first != cycleToUndo) {
        // No cache access in this cycle
        return;
    }

    // It is now safe to undo the cycle at the top of our access stack(s).
    undo();
}

void CacheSim::updateConfiguration() {
    // Cache configuration changed. Reset all state
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

    // Reset the graphical view & processor
    emit configurationChanged();

    if (m_memory.rw || m_memory.rom) {
        // Reload the initial (cycle 0) state of the processor. This is necessary to reflect ie. the instruction which
        // is loaded from the instruction memory in cycle 0.
        processorWasClocked();
    }
}

void CacheSim::processorReset() {
    /** see comment of m_isResetting */
    if (m_isResetting) {
        return;
    }

    m_isResetting = true;
    // The processor might have changed. Since our signals/slot library cannot check for existing connection, we do the
    // safe, slightly redundant, thing of disconnecting and reconnecting the VSRTL design update signals.
    reassociateMemory();
    auto* proc = ProcessorHandler::get()->getProcessorNonConst();
    proc->designWasClocked.Connect(this, &CacheSim::processorWasClocked);
    proc->designWasReversed.Connect(this, &CacheSim::processorWasReversed);
    proc->designWasReset.Connect(this, &CacheSim::processorReset);

    updateConfiguration();
    m_isResetting = false;
}

void CacheSim::setBlocks(unsigned blocks) {
    m_blocks = blocks;
    processorReset();
}
void CacheSim::setLines(unsigned lines) {
    m_lines = lines;
    processorReset();
}
void CacheSim::setWays(unsigned ways) {
    m_ways = ways;
    processorReset();
}

void CacheSim::setWritePolicy(WritePolicy policy) {
    m_wrPolicy = policy;
    processorReset();
}

void CacheSim::setWriteAllocatePolicy(WriteAllocPolicy policy) {
    m_wrAllocPolicy = policy;
    processorReset();
}

void CacheSim::setReplacementPolicy(ReplPolicy policy) {
    m_replPolicy = policy;
    processorReset();
}

void CacheSim::setPreset(const CachePreset& preset) {
    m_blocks = preset.blocks;
    m_ways = preset.ways;
    m_lines = preset.lines;
    m_wrPolicy = preset.wrPolicy;
    m_wrAllocPolicy = preset.wrAllocPolicy;
    m_replPolicy = preset.replPolicy;

    processorReset();
}

}  // namespace Ripes
