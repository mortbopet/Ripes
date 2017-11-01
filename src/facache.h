#ifndef FACACHE_H
#define FACACHE_H

#include "cache.h"

// Fully associative cache
class FACache : public Cache {
public:
  FACache(int size);

  uint32_t readData(uint32_t address) const override;
  void writeData(uint32_t address) override;

private:
  void searchCache(uint32_t address) const;
};

#endif // FACACHE_H
