#pragma once

#include <math.h>
#include <map>
#include <vector>

#include <QObject>

#include "../external/VSRTL/core/vsrtl_register.h"

namespace Ripes {

class CacheSim : public QObject {
    Q_OBJECT
public:
    static constexpr unsigned s_invalidIndex = static_cast<unsigned>(-1);

    enum class WriteAllocPolicy { WriteAllocate, NoWriteAllocate };
    enum class WritePolicy { WriteThrough, WriteBack };
    enum class ReplPolicy { Random, LRU };
    enum class AccessType { Read, Write };

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
        CacheAccessTrace(bool hit) {
            hits = hit ? 1 : 0;
            misses = hit ? 0 : 1;
        }
        CacheAccessTrace(const CacheAccessTrace& pre, AccessType type, bool hit, bool writeback) {
            reads = pre.reads + (type == AccessType::Read ? 1 : 0);
            writes = pre.writes + (type == AccessType::Write ? 1 : 0);
            writebacks = pre.writebacks + (writeback ? 1 : 0);
            hits = pre.hits + (hit ? 1 : 0);
            misses = pre.misses + (hit ? 0 : 1);
        }
    };

    using CacheLine = std::map<unsigned, CacheWay>;

    CacheSim(QObject* parent);

    void setWritePolicy(WritePolicy policy) {
        m_wrPolicy = policy;
        updateConfiguration();
    }

    void setWriteAllocatePolicy(WriteAllocPolicy policy) {
        m_wrAllocPolicy = policy;
        updateConfiguration();
    }

    void setReplacementPolicy(ReplPolicy policy) {
        m_replPolicy = policy;
        updateConfiguration();
    }

    void access(uint32_t address, AccessType type);
    void undo();
    void reset();

    WriteAllocPolicy getWriteAllocPolicy() const { return m_wrAllocPolicy; }
    ReplPolicy getReplacementPolicy() const { return m_replPolicy; }
    WritePolicy getWritePolicy() const { return m_wrPolicy; }

    const std::map<unsigned, CacheAccessTrace>& getAccessTrace() const { return m_accessTrace; }

    double getHitRate() const;
    unsigned getHits() const;
    unsigned getMisses() const;
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

signals:
    void configurationChanged();
    void dataChanged(const CacheTransaction& transaction);
    void wayInvalidated(unsigned lineIdx, unsigned wayIdx);
    void hitrateChanged();

private:
    struct CacheTrace {
        CacheTransaction transaction;
        CacheWay oldWay;
    };

    std::pair<unsigned, CacheSim::CacheWay*> locateEvictionWay(const CacheTransaction& transaction);
    CacheWay evictAndUpdate(CacheTransaction& transaction);
    void analyzeCacheAccess(CacheTransaction& transaction) const;
    void updateConfiguration();
    void updateHitTrace(const CacheTransaction& transaction);

    ReplPolicy m_replPolicy = ReplPolicy::LRU;
    WritePolicy m_wrPolicy = WritePolicy::WriteBack;
    WriteAllocPolicy m_wrAllocPolicy = WriteAllocPolicy::WriteAllocate;

    unsigned m_blockMask = -1;
    unsigned m_lineMask = -1;
    unsigned m_tagMask = -1;

    int m_blocks = 1;  // Some power of 2
    int m_lines = 2;   // Some power of 2
    int m_ways = 2;    // Some power of 2

    std::map<unsigned, CacheLine> m_cacheLines;

    void updateCacheLineReplFields(CacheLine& line, unsigned wayIdx);
    /**
     * @brief revertCacheLineReplFields
     * Called whenever undoing a transaction to the cache. Reverts a cacheline's replacement fields according to the
     * configured replacement policy.
     */
    void revertCacheLineReplFields(CacheLine& line, const CacheWay& oldWay, unsigned wayIdx);

    std::map<unsigned, CacheAccessTrace> m_accessTrace;

    // Cache access trace
    // The following information is used to track all most-recent modifications (up to a given set constant) made to the
    // stack. Storing all modifications allows us to rollback any changes performed to the cache, when clock cycles are
    // undone.
    std::deque<CacheTrace> m_traceStack;
    CacheTrace popTrace();
    void pushTrace(const CacheTrace& trace);
};

static std::map<CacheSim::ReplPolicy, QString> s_cacheReplPolicyStrings{{CacheSim::ReplPolicy::Random, "Random"},
                                                                        {CacheSim::ReplPolicy::LRU, "LRU"}};
static std::map<CacheSim::WriteAllocPolicy, QString> s_cacheWriteAllocateStrings{
    {CacheSim::WriteAllocPolicy::WriteAllocate, "Write allocate"},
    {CacheSim::WriteAllocPolicy::NoWriteAllocate, "No write allocate"}};

static std::map<CacheSim::WritePolicy, QString> s_cacheWritePolicyStrings{
    {CacheSim::WritePolicy::WriteThrough, "Write-through"},
    {CacheSim::WritePolicy::WriteBack, "Write-back"}};

}  // namespace Ripes
