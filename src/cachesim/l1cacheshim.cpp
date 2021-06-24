#include "l1cacheshim.h"

#include "processorhandler.h"

namespace Ripes {

L1CacheShim::L1CacheShim(CacheType type, QObject* parent) : CacheInterface(parent), m_type(type) {
    connect(ProcessorHandler::get(), &ProcessorHandler::processorReset, this, &L1CacheShim::processorReset);

    // We must update the cache statistics on each cycle, in lockstep with the procsesor itself. Connect to
    // ProcessorHandler::processorClocked and ensure that the handler is executed in the thread that the processor lives
    // in (direct connection).
    connect(ProcessorHandler::get(), &ProcessorHandler::processorClocked, this, &L1CacheShim::processorWasClocked,
            Qt::DirectConnection);
    connect(ProcessorHandler::get(), &ProcessorHandler::processorReversed, this, &L1CacheShim::processorReversed);

    processorReset();
}

void L1CacheShim::access(AInt, MemoryAccess::Type) {
    // Should never occur; the shim determines accesses based on investigating the associated memory.
    Q_ASSERT(false);
}

void L1CacheShim::processorReset() {
    // Propagate a reset through the cache hierarchy
    CacheInterface::reset();

    if (m_nextLevelCache) {
        // Reload the initial (cycle 0) state of the processor. This is necessary to reflect ie. the instruction which
        // is loaded from the instruction memory in cycle 0.
        processorWasClocked();
    }
}

void L1CacheShim::processorReversed() {
    // Start propagating a reverse call through the cache hierarchy
    CacheInterface::reverse();
}

void L1CacheShim::processorWasClocked() {
    if (m_type == CacheType::DataCache) {
        const auto dataAccess = ProcessorHandler::getProcessor()->dataMemAccess();

        // Determine whether the memory is being accessed in the current cycle, and if so, the access type.
        switch (dataAccess.type) {
            case MemoryAccess::Write:
                m_nextLevelCache->access(dataAccess.address, MemoryAccess::Write);
                break;
            case MemoryAccess::Read:
                m_nextLevelCache->access(dataAccess.address, MemoryAccess::Read);
                break;
            case MemoryAccess::None:
            default:
                break;
        }
    } else {
        const auto instrAccess = ProcessorHandler::getProcessor()->instrMemAccess();
        if (instrAccess.type == MemoryAccess::Read) {
            m_nextLevelCache->access(instrAccess.address, MemoryAccess::Read);
        }
    }
}

}  // namespace Ripes
