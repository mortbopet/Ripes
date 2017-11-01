#ifndef DMCACHE_H
#define DMCACHE_H

#include "cache.h"
#include <cstdint>

// Direct mapped cache
class DMCache : public Cache {
public:
  DMCache(int size);

  uint32_t readData(uint32_t address) const override;
  void writeData(uint32_t address) override;

private:
  void searchCache(uint32_t address) const;
};

#endif // DMCACHE_H
