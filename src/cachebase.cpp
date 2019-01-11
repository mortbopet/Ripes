#include "cachebase.h"
#include <cassert>

CacheBase::CacheBase(CacheProperties properties, int* cycleCounterPtr) {
    // Initialize cache of size, set to zeros
    m_properties = properties;
}

void CacheBase::assertSize(int size) const {
    // Assert cache size is a power of 2
    assert((size & (size - 1)) == 0);
}
