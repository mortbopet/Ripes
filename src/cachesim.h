#pragma once

#include <math.h>
#include <map>
#include <vector>

#include <QObject>

namespace Ripes {

class CacheSim : public QObject {
    Q_OBJECT
public:
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
    };

    struct CacheTransaction {
        uint32_t address;
        unsigned lineIdx = -1;
        unsigned wayIdx = -1;
        unsigned blockIdx = -1;
        unsigned tag = -1;

        bool isHit = false;
        bool read = false;
        bool write = false;
        bool transToValid = false;  // True if the cacheline just transitioned from invalid to valid
        bool tagChanged = false;    // True if transToValid or the previous entry was evicted
    };

    struct CacheWay {
        uint32_t tag;
        // std::map<unsigned, uint32_t> blocks; we do not store the actual data; no reason to!
        bool dirty = false;
        bool valid = false;

        // LRU algorithm relies on invalid cache ways to have an initial high value. -1 ensures maximum value for all
        // way sizes.
        unsigned lru = -1;
    };

    using CacheLine = std::map<unsigned, CacheWay>;

    CacheSim(QObject* parent);

    void setWritePolicy(WritePolicy policy) {
        m_wrPolicy = policy;
        updateConfiguration();
    }

    void setReplacementPolicy(ReplPolicy policy) {
        m_replPolicy = policy;
        updateConfiguration();
    }

    void access(uint32_t address, AccessType type);
    void undo();
    void reset();

    ReplPolicy getReplacementPolicy() const { return m_replPolicy; }
    WritePolicy getWritePolicy() const { return m_wrPolicy; }

    double getHitRate() const { return m_hitrate; }
    CacheSize getCacheSize() const;

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
    void hitRateChanged(double hitrate);

private:
    void updateHitRate();
    void evictAndUpdate(CacheTransaction& transaction);
    void analyzeCacheAccess(CacheTransaction& transaction);
    void updateConfiguration();

    ReplPolicy m_replPolicy = ReplPolicy::LRU;
    WritePolicy m_wrPolicy = WritePolicy::WriteBack;

    unsigned m_blockMask = -1;
    unsigned m_lineMask = -1;
    unsigned m_tagMask = -1;

    int m_blocks = 1;  // Some power of 2
    int m_lines = 2;   // Some power of 2
    int m_ways = 2;    // Some power of 2

    double m_hitrate = 0;  // Most recent hitrate

    std::map<unsigned, CacheLine> m_cacheLines;

    void updateCacheLineLRU(CacheLine& line, unsigned lruIdx);

    struct CacheAccessTrace {
        int hits = 0;
        int misses = 0;
        CacheAccessTrace() {}
        CacheAccessTrace(bool hit) {
            hits = hit ? 1 : 0;
            misses = hit ? 0 : 1;
        }
        CacheAccessTrace(const CacheAccessTrace& pre, bool hit) {
            hits = pre.hits + (hit ? 1 : 0);
            misses = pre.misses + (hit ? 0 : 1);
        }
    };
    std::map<unsigned, CacheAccessTrace> m_accessTrace;
};

static std::map<CacheSim::ReplPolicy, QString> s_cachePolicyStrings{{CacheSim::ReplPolicy::Random, "Random"},
                                                                    {CacheSim::ReplPolicy::LRU, "LRU"}};

}  // namespace Ripes
