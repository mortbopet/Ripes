#include "cache.h"

#include <assert.h>

Cache::Cache(int size, int missPenalty, int searchPenalty) {

  // Assert cache size is a power of 2
  assert((size & (size - 1)) == 0);

  // Initialize cache of size, set to zeros
  m_cache = std::vector<uint32_t>(size, 0);
  m_missPenalty = missPenalty;
  m_searchPenalty = searchPenalty;
}

void Cache::resize(int size) {
  // Only resize if size is power of 2
  if ((size & (size - 1)) == 0) {
    m_cache.resize(size);
  }
}
