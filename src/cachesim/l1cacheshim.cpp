#include "l1cacheshim.h"

#include "processorhandler.h"

namespace Ripes {

L1CacheShim::L1CacheShim(CacheType type, QObject* parent) : CacheInterface(parent), m_type(type) {
    reassociateMemory();
    connect(ProcessorHandler::get(), &ProcessorHandler::reqProcessorReset, this, &L1CacheShim::processorReset);
    processorReset();
}

void L1CacheShim::access(uint32_t, AccessType) {
    // Should never occur; the shim determines accesses based on investigating the associated memory.
    Q_ASSERT(false);
}

void L1CacheShim::processorReset() {
    auto* proc = ProcessorHandler::getProcessorNonConst();
    reassociateMemory();
    proc->designWasClocked.Connect(this, &L1CacheShim::processorWasClocked);
    proc->designWasReset.Connect(this, &L1CacheShim::processorReset);
    proc->designWasReversed.Connect(this, &L1CacheShim::processorReversed);

    // Propagate a reset through the cache hierarchy
    CacheInterface::reset();

    if ((m_memory.rw || m_memory.rom) && m_nextLevelCache.lock()) {
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
        switch (m_memory.rw->op.uValue()) {
            case MemOp::SB:
            case MemOp::SH:
            case MemOp::SW:
                if (m_memory.rw->wr_en.uValue() == 1) {
                    m_nextLevelCache.lock()->access(m_memory.rw->addr.uValue(), AccessType::Write);
                }
                break;
            case MemOp::LB:
            case MemOp::LBU:
            case MemOp::LH:
            case MemOp::LHU:
            case MemOp::LW:
                m_nextLevelCache.lock()->access(m_memory.rw->addr.uValue(), AccessType::Read);
                break;
            case MemOp::NOP:
            default:
                break;
        }
    } else {
        m_nextLevelCache.lock()->access(m_memory.rom->addr.uValue(), AccessType::Read);
    }
}

void L1CacheShim::reassociateMemory() {
    if (m_type == CacheType::DataCache) {
        m_memory.rw = ProcessorHandler::getDataMemory();
        Q_ASSERT(m_memory.rw != nullptr);
    } else if (m_type == CacheType::InstrCache) {
        m_memory.rom = ProcessorHandler::getInstrMemory();
        Q_ASSERT(m_memory.rom != nullptr);
    } else {
        Q_ASSERT(false);
    }
}

}  // namespace Ripes
