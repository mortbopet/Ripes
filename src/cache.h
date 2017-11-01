#ifndef CACHE_H
#define CACHE_H

#include <cstdint>
#include <vector>

class Cache {
public:
  Cache(int missPenalty = 0, int searchPenalty = 0);
  virtual uint32_t readData(uint32_t address) = 0;
  virtual void writeData(uint32_t address) = 0;

  void setMissPenalty(int cycles) { m_missPenalty = cycles; }
  void setSearchPenalty(int cycles) { m_searchPenalty = cycles; }

  virtual void resize(int size) = 0;

protected:
  void assertSize(int size) const;
  int m_readCount = 0;
  int m_missCount = 0;

private:
  int m_missPenalty;
  int m_searchPenalty;
  uint32_t *m_memoryPtr;
};

#endif // CACHE_H
