#include "cachebase.h"
#include "binutils.h"

#include <random>

namespace Ripes {

void CacheBase::updateCacheValue(uint32_t address, uint32_t value) {
    const unsigned lineIdx = getAccessLineIdx();
    const unsigned blockIdx = getAccessBlockIdx();

    auto& cacheLine = m_cacheLines[lineIdx];

    // Update based on replacement policy
    if (m_policy == CacheReplPlcy::Random) {
        // Select a random way
        const unsigned wayIdx = std::rand() % getSets();
        auto& way = cacheLine[wayIdx];
        // Todo: this is not valid; the entire cache line (all blocks) should be read
        way.valid = true;
        emit dataChanged(address);
    }
}

bool CacheBase::checkCacheHit() {
    const unsigned lineIdx = getAccessLineIdx();

    if (m_cacheLines.count(lineIdx) != 0) {
        int setIdx = 0;
        for (const auto& way : m_cacheLines.at(lineIdx)) {
            if (way.second.tag == getAccessTag()) {
                m_currentSetIdx = setIdx;
                return true;
            }
            setIdx++;
        }
    }
    return false;
}

void CacheBase::read(uint32_t address) {
    m_currentAccessAddress = address;
    m_currentAccessIsHit = checkCacheHit();
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

unsigned CacheBase::getAccessSetIdx() const {
    return m_currentSetIdx;
}

unsigned CacheBase::getAccessTag() const {
    uint32_t maskedAddress = m_currentAccessAddress & m_lineMask;
    maskedAddress >>= 2 + getBlockBits() + getLineBits();
    return maskedAddress;
}

unsigned CacheBase::getAccessBlockIdx() const {
    uint32_t maskedAddress = m_currentAccessAddress & m_blockMask;
    maskedAddress >>= 2;
    return maskedAddress;
}

void CacheBase::updateConfiguration() {
    // Recalculate masks
    int bitoffset = 2;  // 2^2 = 4-byte offset (32-bit words in cache)

    m_blockMask = generateBitmask(getBlockBits()) << bitoffset;
    bitoffset += getBlockBits();

    m_lineMask = generateBitmask(getLineBits()) << bitoffset;
    bitoffset += getLineBits();

    m_tagMask = generateBitmask(32 - bitoffset) << bitoffset;

    emit configurationChanged();
}

}  // namespace Ripes
