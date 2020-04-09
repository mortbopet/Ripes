#pragma once

#include <math.h>
#include <map>
#include <vector>

#include <QObject>

namespace Ripes {

enum class CacheReplPlcy { Random, LRU };
static std::map<CacheReplPlcy, QString> CachePolicyStrings{{CacheReplPlcy::Random, "Random"},
                                                           {CacheReplPlcy::LRU, "LRU"}};

struct CacheSize {
    unsigned bits = 0;
    std::vector<QString> components;
};

class CacheBase : public QObject {
    Q_OBJECT
public:
    CacheBase(QObject* parent) : QObject(parent) {}

    void setReplacementPolicy(CacheReplPlcy policy) {
        m_policy = policy;
        updateConfiguration();
    }

    void read(uint32_t address);
    void write(uint32_t address);
    void undo();

    CacheReplPlcy getReplacementPolicy() const { return m_policy; }

    double getHitRate() const;
    CacheSize getCacheSize() const;

    int getBlockBits() const { return m_blocks; }
    int getWaysBits() const { return m_ways; }
    int getLineBits() const { return m_lines; }

    int getBlocks() const { return static_cast<int>(std::pow(2, m_blocks)); }
    int getWays() const { return static_cast<int>(std::pow(2, m_ways)); }
    int getLines() const { return static_cast<int>(std::pow(2, m_lines)); }

    unsigned getAccessLineIdx() const;
    unsigned getAccessWayIdx() const;
    unsigned getAccessBlockIdx() const;
    unsigned getAccessTag() const;

    bool isCacheHit() const { return m_currentAccessIsHit; }

public slots:
    void setBlocks(unsigned blocks) {
        m_blocks = blocks;
        updateConfiguration();
    }
    void setLines(unsigned lines) {
        m_lines = lines;
        updateConfiguration();
    }
    void setWays(unsigned ways) {
        m_ways = ways;
        updateConfiguration();
    }

signals:
    void configurationChanged();
    void accessChanged(bool active);
    void dataChanged(uint32_t address);
    void hitRateChanged(double hitrate);

private:
    void updateHitRate();
    void updateCacheValue(uint32_t address);
    void analyzeCacheAccess();
    void updateConfiguration();

    CacheReplPlcy m_policy = CacheReplPlcy::LRU;
    uint32_t m_currentAccessAddress;
    bool m_currentAccessIsHit;
    unsigned m_currentWayIdx;

    unsigned m_blockMask;
    unsigned m_lineMask;
    unsigned m_tagMask;

    int m_blocks = 1;  // Some power of 2
    int m_lines = 2;   // Some power of 2
    int m_ways = 2;    // Some power of 2

    double m_hitrate;  // Most recent hitrate

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
    std::map<unsigned, CacheLine> m_cacheLines;

    void updateCacheLineLRU(CacheLine& line, unsigned lruIdx);
    CacheLine const* m_currentAccessLine = nullptr;

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

public:
    const CacheLine* getCurrentAccessLine() const { return m_currentAccessLine; }
};

}  // namespace Ripes
