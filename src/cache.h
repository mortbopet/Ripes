#ifndef CACHE_H
#define CACHE_H

#include <cstdint>
#include <vector>

class Cache {
public:
  Cache(int size, int missPenalty = 0, int searchPenalty = 0);
  virtual uint32_t readData(uint32_t address) const = 0;
  virtual void writeData(uint32_t address) = 0;

  void setMissPenalty(int cycles) { m_missPenalty = cycles; }
  void setSearchPenalty(int cycles) { m_searchPenalty = cycles; }

  void resize(int size);

private:
  std::vector<uint32_t> m_cache;
  int m_missPenalty;
  int m_searchPenalty;
  uint32_t *m_memoryPtr;
};

#endif // CACHE_H
