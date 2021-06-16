#include "l1cacheshim.h"

#include "processorhandler.h"

namespace Ripes {

L1CacheShim::L1CacheShim(CacheType type, QObject* parent) : CacheInterface(parent), m_type(type) {
    reassociateMemory();
    connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this, &L1CacheShim::reassociateMemory);
    connect(ProcessorHandler::get(), &ProcessorHandler::processorReset, this, &L1CacheShim::processorReset);
    connect(ProcessorHandler::get(), &ProcessorHandler::processorClocked, this, &L1CacheShim::processorWasClocked,
            Qt::DirectConnection);
    connect(ProcessorHandler::get(), &ProcessorHandler::processorReversed, this, &L1CacheShim::processorReversed);

    processorReset();
}

void L1CacheShim::access(uint32_t, AccessType) {
    // Should never occur; the shim determines accesses based on investigating the associated memory.
    Q_ASSERT(false);
}

void L1CacheShim::processorReset() {
    // Propagate a reset through the cache hierarchy
    CacheInterface::reset();

    if (m_memory && m_nextLevelCache) {
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
        // Determine whether the memory is being accessed in the current cycle, and if so, the access type.
        switch (m_memory->opSig()) {
            case MemOp::SB:
            case MemOp::SH:
            case MemOp::SW:
                if (m_memory->wrEnSig() == 1) {
                    m_nextLevelCache->access(m_memory->addressSig(), AccessType::Write);
                }
                break;
            case MemOp::LB:
            case MemOp::LBU:
            case MemOp::LH:
            case MemOp::LHU:
            case MemOp::LW:
                m_nextLevelCache->access(m_memory->addressSig(), AccessType::Read);
                break;
            case MemOp::NOP:
            default:
                break;
        }
    } else {
        m_nextLevelCache->access(m_memory->addressSig(), AccessType::Read);
    }
}

void L1CacheShim::reassociateMemory() {
    if (m_type == CacheType::DataCache) {
        m_memory = ProcessorHandler::getDataMemory();
    } else if (m_type == CacheType::InstrCache) {
        m_memory = ProcessorHandler::getInstrMemory();
    } else {
        Q_ASSERT(false);
    }
    Q_ASSERT(m_memory != nullptr);
}

}  // namespace Ripes
