#pragma once

#include <math.h>
#include <map>
#include <vector>

#include <QObject>

#include "../external/VSRTL/core/vsrtl_register.h"
#include "processors/RISC-V/rv_memory.h"

using RWMemory = vsrtl::core::RVMemory<32, 32>;
using ROMMemory = vsrtl::core::ROM<32, 32>;

namespace Ripes {

class CacheSim : public QObject {
    Q_OBJECT
public:
    static constexpr unsigned s_invalidIndex = static_cast<unsigned>(-1);

    enum class WriteAllocPolicy { WriteAllocate, NoWriteAllocate };
    enum class WritePolicy { WriteThrough, WriteBack };
    enum class ReplPolicy { Random, LRU };
    enum class AccessType { Read, Write };
    enum class CacheType { DataCache, InstrCache };

    struct CacheSize {
        unsigned bits = 0;
        std::vector<QString> components;
    };

    struct CachePreset {
        int blocks;
        int lines;
        int ways;

        WritePolicy wrPolicy;
        WriteAllocPolicy wrAllocPolicy;
        ReplPolicy replPolicy;
    };

    struct CacheWay {
        uint32_t tag = -1;
        std::set<unsigned> dirtyBlocks;
        bool dirty = false;
        bool valid = false;

        // LRU algorithm relies on invalid cache ways to have an initial high value. -1 ensures maximum value for all
        // way sizes.
        unsigned lru = -1;
    };

    struct CacheIndex {
        unsigned line = s_invalidIndex;
        unsigned way = s_invalidIndex;
        unsigned block = s_invalidIndex;
        void assertValid() const {
            Q_ASSERT(line != s_invalidIndex && "Cache line index is invalid");
            Q_ASSERT(way != s_invalidIndex && "Cache way index is invalid");
            Q_ASSERT(block != s_invalidIndex && "Cache block index is invalid");
        }
    };

    struct CacheTransaction {
        uint32_t address;
        CacheIndex index;

        bool isHit = false;
        bool isWriteback = false;  // True if the transaction resulted in an eviction of a dirty cacheline
        AccessType type;
        bool transToValid = false;  // True if the cacheline just transitioned from invalid to valid
        bool tagChanged = false;    // True if transToValid or the previous entry was evicted
    };

    struct CacheAccessTrace {
        int hits = 0;
        int misses = 0;
        int reads = 0;
        int writes = 0;
        int writebacks = 0;
        CacheAccessTrace() {}
        CacheAccessTrace(const CacheTransaction& transaction) : CacheAccessTrace(CacheAccessTrace(), transaction) {}
        CacheAccessTrace(const CacheAccessTrace& pre, const CacheTransaction& transaction) {
            reads = pre.reads + (transaction.type == AccessType::Read ? 1 : 0);
            writes = pre.writes + (transaction.type == AccessType::Write ? 1 : 0);
            writebacks = pre.writebacks + (transaction.isWriteback ? 1 : 0);
            hits = pre.hits + (transaction.isHit ? 1 : 0);
            misses = pre.misses + (transaction.isHit ? 0 : 1);
        }
    };

    using CacheLine = std::map<unsigned, CacheWay>;

    CacheSim(QObject* parent);
    void setType(CacheType type);
    void setWritePolicy(WritePolicy policy);
    void setWriteAllocatePolicy(WriteAllocPolicy policy);
    void setReplacementPolicy(ReplPolicy policy);

    void access(uint32_t address, AccessType type);
    void undo();
    void processorReset();

    WriteAllocPolicy getWriteAllocPolicy() const { return m_wrAllocPolicy; }
    ReplPolicy getReplacementPolicy() const { return m_replPolicy; }
    WritePolicy getWritePolicy() const { return m_wrPolicy; }

    const std::map<unsigned, CacheAccessTrace>& getAccessTrace() const { return m_accessTrace; }

    double getHitRate() const;
    unsigned getHits() const;
    unsigned getMisses() const;
    unsigned getWritebacks() const;
    CacheSize getCacheSize() const;

    uint32_t buildAddress(unsigned tag, unsigned lineIdx, unsigned blockIdx) const;

    int getBlockBits() const { return m_blocks; }
    int getWaysBits() const { return m_ways; }
    int getLineBits() const { return m_lines; }
    int getTagBits() const { return 32 - 2 /*byte offset*/ - getBlockBits() - getLineBits(); }

    int getBlocks() const { return static_cast<int>(std::pow(2, m_blocks)); }
    int getWays() const { return static_cast<int>(std::pow(2, m_ways)); }
    int getLines() const { return static_cast<int>(std::pow(2, m_lines)); }
    unsigned getBlockMask() const { return m_blockMask; }
    unsigned getTagMask() const { return m_tagMask; }
    unsigned getLineMask() const { return m_lineMask; }

    unsigned getLineIdx(const uint32_t address) const;
    unsigned getBlockIdx(const uint32_t address) const;
    unsigned getTag(const uint32_t address) const;

    const CacheLine* getLine(unsigned idx) const;

public slots:
    void setBlocks(unsigned blocks);
    void setLines(unsigned lines);
    void setWays(unsigned ways);
    void setPreset(const CachePreset& preset);

    /**
     * @brief processorWasClocked/processorWasReversed
     * Slot functions for clocked/Reversed signals emitted by the currently attached processor.
     */
    void processorWasClocked();
    void processorWasReversed();

signals:
    void configurationChanged();
    void dataChanged(const CacheTransaction* transaction);
    void hitrateChanged();

    // Signals that the entire cache line @p
    /**
     * @brief wayInvalidated
     * Signals that all ways in the cacheline @param lineIdx which contains way @param wayIdx should be invalidated in
     * the graphical view.
     */
    void wayInvalidated(unsigned lineIdx, unsigned wayIdx);

    /**
     * @brief cacheInvalidated
     * Signals that all cachelines in the cache should be invalidated in the graphical view
     */
    void cacheInvalidated();

private:
    struct CacheTrace {
        CacheTransaction transaction;
        CacheWay oldWay;
    };

    std::pair<unsigned, CacheSim::CacheWay*> locateEvictionWay(const CacheTransaction& transaction);
    CacheWay evictAndUpdate(CacheTransaction& transaction);
    void analyzeCacheAccess(CacheTransaction& transaction) const;
    void updateConfiguration();
    void pushAccessTrace(const CacheTransaction& transaction);
    void popAccessTrace();
    /**
     * @brief isAsynchronouslyAccessed
     * If the processor is in its 'running' state, it is currently being executed in a separate thread. In this case,
     * cache accessing is also performed asynchronously, and we do not want to perform any signalling to the GUI (the
     * entirety of the graphical representation of the cache is invalidated and redrawn upon asynchronous running
     * finishing).
     */
    bool isAsynchronouslyAccessed() const;

    /**
     * @brief reassociateMemory
     * Binds to a memory component exposed by the processor handler, based on the current cache type.
     */
    void reassociateMemory();

    ReplPolicy m_replPolicy = ReplPolicy::LRU;
    WritePolicy m_wrPolicy = WritePolicy::WriteBack;
    WriteAllocPolicy m_wrAllocPolicy = WriteAllocPolicy::WriteAllocate;

    unsigned m_blockMask = -1;
    unsigned m_lineMask = -1;
    unsigned m_tagMask = -1;

    int m_blocks = 2;  // Some power of 2
    int m_lines = 5;   // Some power of 2
    int m_ways = 0;    // Some power of 2

    /**
     * @brief m_memory
     * The cache simulator may be attached to either a ROM or a Read/Write memory element. Accessing the underlying
     * VSRTL component signals are dependent on the given type of the memory.
     */
    CacheType m_type;
    union {
        RWMemory const* rw = nullptr;
        ROMMemory const* rom;

    } m_memory;

    /**
     * @brief m_cacheLines
     * The datastructure for storing our cache hierachy, as per the current cache configuration.
     */
    std::map<unsigned, CacheLine> m_cacheLines;

    void updateCacheLineReplFields(CacheLine& line, unsigned wayIdx);
    /**
     * @brief revertCacheLineReplFields
     * Called whenever undoing a transaction to the cache. Reverts a cacheline's replacement fields according to the
     * configured replacement policy.
     */
    void revertCacheLineReplFields(CacheLine& line, const CacheWay& oldWay, unsigned wayIdx);

    /**
     * @brief m_accessTrace
     * The access trace stack contains cache access statistics for each simulation cycle. Contrary to the TraceStack
     * (m_traceStack).
     */
    std::map<unsigned, CacheAccessTrace> m_accessTrace;

    /**
     * @brief m_traceStack
     * The following information is used to track all most-recent modifications made to the stack. The stack is of a
     * fixed sized which is equal to the undo stack of VSRTL memory elements. Storing all modifications allows us to
     * rollback any changes performed to the cache, when clock cycles are undone.
     */
    std::deque<CacheTrace> m_traceStack;

    /**
     * @brief m_isResetting
     * The cacheSim can be reset by either internally modyfing cache configuration parameters or externally through a
     * processor reset. Given that modifying the cache parameters itself will prompt a reset of the processor, we need a
     * way to distinquish whether a processor reset request originated from an internal cache configuration change. If
     * so, we do not emit a processor request signal, avoiding a signalling loop.
     */
    bool m_isResetting = false;

    CacheTrace popTrace();
    void pushTrace(const CacheTrace& trace);
};

const static std::map<CacheSim::ReplPolicy, QString> s_cacheReplPolicyStrings{{CacheSim::ReplPolicy::Random, "Random"},
                                                                              {CacheSim::ReplPolicy::LRU, "LRU"}};
const static std::map<CacheSim::WriteAllocPolicy, QString> s_cacheWriteAllocateStrings{
    {CacheSim::WriteAllocPolicy::WriteAllocate, "Write allocate"},
    {CacheSim::WriteAllocPolicy::NoWriteAllocate, "No write allocate"}};

const static std::map<CacheSim::WritePolicy, QString> s_cacheWritePolicyStrings{
    {CacheSim::WritePolicy::WriteThrough, "Write-through"},
    {CacheSim::WritePolicy::WriteBack, "Write-back"}};

}  // namespace Ripes
