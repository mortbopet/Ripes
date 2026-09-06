#pragma once

#include <QObject>

#include "cachesim.h"

#include "VSRTL/core/vsrtl_memory.h"
#include "isa/isa_types.h"

namespace Ripes {

/**
 * @brief The CacheShim class
 * Provides a wrapper around the current processor models' data- and instruction
 * memories, to be used in the cache simulator interface.
 */
class L1CacheShim : public CacheInterface {
  Q_OBJECT
public:
  enum class CacheType { DataCache, InstrCache };
  L1CacheShim(CacheType type, QObject *parent);
  void access(AInt address, MemoryAccess::Type type) override;

  void setType(CacheType type);

  /**
   * @brief setRequireCacheInterface
   * When true (the default), the cache is only simulated while the current
   * processor exposes the corresponding cache interface (see
   * RipesProcessor::Features). Models without a cache interface - such as the
   * ISA simulator - then incur no cache-simulation cost. Set to false to force
   * simulation regardless (e.g. the CLI --cache option, which explicitly
   * requests cache statistics for any processor).
   */
  void setRequireCacheInterface(bool require) {
    m_requireCacheInterface = require;
  }

private:
  void processorReset();
  void processorWasClocked();
  void processorReversed();

  /**
   * @brief m_memory
   * The cache simulator may be attached to either a ROM or a Read/Write memory
   * element. Accessing the underlying VSRTL component signals are dependent on
   * the given type of the memory.
   */
  CacheType m_type;

  bool m_requireCacheInterface = true;
};

} // namespace Ripes
