#pragma once

#include <math.h>
#include <map>
#include <vector>

#include <QObject>

namespace Ripes {

enum class CacheReplPlcy { Random, LRU };
static std::map<CacheReplPlcy, QString> CachePolicyStrings{{CacheReplPlcy::Random, "Random"},
                                                           {CacheReplPlcy::LRU, "LRU"}};

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

    int getSizeInBits();

    int getBlockBits() const { return m_blocks; }
    int getSetBits() const { return m_sets; }
    int getLineBits() const { return m_lines; }

    int getBlocks() const { return static_cast<int>(std::pow(2, m_blocks)); }
    int getSets() const { return static_cast<int>(std::pow(2, m_sets)); }
    int getLines() const { return static_cast<int>(std::pow(2, m_lines)); }

    unsigned getAccessLineIdx() const;
    unsigned getAccessSetIdx() const;
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
    void setSets(unsigned sets) {
        m_sets = sets;
        updateConfiguration();
    }

signals:
    void configurationChanged();
    void accessChanged(bool active);
    void dataChanged(uint32_t address);

private:
    void updateCacheValue(uint32_t address);
    bool analyzeCacheHit();
    void updateConfiguration();

    CacheReplPlcy m_policy = CacheReplPlcy::LRU;
    uint32_t m_currentAccessAddress;
    bool m_currentAccessIsHit;
    unsigned m_currentSetIdx;

    unsigned m_blockMask;
    unsigned m_lineMask;
    unsigned m_tagMask;

    int m_blocks = 1;  // Some power of 2
    int m_lines = 2;   // Some power of 2
    int m_sets = 2;    // Some power of 2

    struct CacheWay {
        uint32_t tag;
        // std::map<unsigned, uint32_t> blocks; we do not store the actual data; no reason to!
        bool dirty = false;
        bool valid = false;

        // LRU algorithm relies on invalid cache ways to have an initial high value. -1 ensures maximum value for all
        // set sizes.
        unsigned lru = -1;
    };

    using CacheLine = std::map<unsigned, CacheWay>;
    std::map<unsigned, CacheLine> m_cacheLines;

    void updateCacheLineLRU(CacheLine& line, unsigned lruIdx);
    CacheLine const* m_currentAccessLine = nullptr;

public:
    const CacheLine* getCurrentAccessLine() const { return m_currentAccessLine; }
};

}  // namespace Ripes
