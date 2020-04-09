#pragma once

#include <math.h>
#include <map>
#include <vector>

#include <QObject>

namespace Ripes {

class CacheBase : public QObject {
    Q_OBJECT
public:
    struct CacheLine {
        uint32_t tag;
        std::vector<uint32_t> blocks;
        bool hasDirty = false;
        bool hasValid = true;
    };
    using CacheSet = std::map<unsigned, CacheLine>;

    CacheBase(QObject* parent) : QObject(parent) {}

    void read(uint32_t address);
    void write(uint32_t address);
    void undo();

    int getSizeInBits();

    int getBlockBits() const { return m_blocks; }
    int getSetBits() const { return m_sets; }
    int getLineBits() const { return m_lines; }

    int getBlocks() const { return static_cast<int>(std::pow(2, m_blocks)); }
    int getSets() const { return static_cast<int>(std::pow(2, m_sets)); }
    int getLines() const { return static_cast<int>(std::pow(2, m_lines)); }

public slots:
    void setBlocks(unsigned blocks) {
        m_blocks = blocks;
        emit parametersChanged();
    }
    void setLines(unsigned lines) {
        m_lines = lines;
        emit parametersChanged();
    }
    void setSets(unsigned sets) {
        m_sets = sets;
        emit parametersChanged();
    }

signals:
    void parametersChanged();

private:
    int m_blocks = 1;  // Some power of 2
    int m_lines = 4;   // Some power of 2
    int m_sets = 0;    // Some power of 2

    std::map<unsigned, CacheSet> m_cacheSets;
};

}  // namespace Ripes
