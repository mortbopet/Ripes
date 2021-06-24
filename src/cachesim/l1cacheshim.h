#pragma once

#include <QObject>

#include "cachesim.h"

#include "VSRTL/core/vsrtl_memory.h"
#include "ripes_types.h"

namespace Ripes {

/**
 * @brief The CacheShim class
 * Provides a wrapper around the current processor models' data- and instruction memories, to be used in the cache
 * simulator interface.
 */
class L1CacheShim : public CacheInterface {
    Q_OBJECT
public:
    enum class CacheType { DataCache, InstrCache };
    L1CacheShim(CacheType type, QObject* parent);
    void access(AInt address, MemoryAccess::Type type) override;

    void setType(CacheType type);

private:
    void processorReset();
    void processorWasClocked();
    void processorReversed();

    /**
     * @brief m_memory
     * The cache simulator may be attached to either a ROM or a Read/Write memory element. Accessing the underlying
     * VSRTL component signals are dependent on the given type of the memory.
     */
    CacheType m_type;
};

}  // namespace Ripes
