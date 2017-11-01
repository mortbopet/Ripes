#include "cache.h"
#include <assert.h>

Cache::Cache(int missPenalty, int searchPenalty) {
  // Initialize cache of size, set to zeros
  m_missPenalty = missPenalty;
  m_searchPenalty = searchPenalty;
}

void Cache::assertSize(int size) const {
  // Assert cache size is a power of 2
  assert((size & (size - 1)) == 0);
}
