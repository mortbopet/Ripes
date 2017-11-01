#include "facache.h"

FACache::FACache(int size) : Cache(size) {}

uint32_t FACache::readData(uint32_t address) const { return 0; }

void FACache::writeData(uint32_t address) {}

void FACache::searchCache(uint32_t address) const {}
