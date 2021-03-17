#include "iomanager.h"

#include "processorhandler.h"
#include "ripessettings.h"

#include "VSRTL/core/vsrtl_sparsearray.h"

namespace Ripes {

IOManager::IOManager() : QObject(nullptr) {
    // Always re-register the currently active peripherals when the processor changes
    connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
            &IOManager::refreshAllPeriphsToProcessor);
    connect(ProcessorHandler::get(), &ProcessorHandler::reqProcessorReset, this, &IOManager::refreshMemoryMap);
    refreshMemoryMap();
}

uint32_t IOManager::nextPeripheralAddress() const {
    uint32_t base = 0;
    if (m_peripherals.empty()) {
        base = RipesSettings::value(RIPES_SETTING_PERIPHERALS_START).toUInt();
    } else {
        for (const auto& periph : m_peripherals) {
            if (periph.second.end() > base) {
                base = periph.second.end();
            }
        }
    }
    return base;
}

void IOManager::peripheralSizeChanged(IOBase* peripheral) {
    /** @todo
     * - Check new peripheral size compared to old. If change detected, we must re-create the memory map to not have any
     * overlap.
     * - if not the above, re-register the peripheral with the processor; peripheral size may have decreased aswell.
     * - update size of peripheral in internal map
     */
    Q_ASSERT(false);
}

void IOManager::registerPeripheralWithProcessor(IOBase* peripheral) {
    ProcessorHandler::get()->getMemory().addRegion(
        m_peripherals.at(peripheral).startAddr, peripheral->size(),
        vsrtl::core::IOFunctors{
            [peripheral](uint32_t offset, uint32_t value, uint32_t size) { peripheral->ioWrite(offset, value, size); },
            [peripheral](uint32_t offset, uint32_t size) { return peripheral->ioRead(offset, size); }});
}

void IOManager::unregisterPeripheralWithProcessor(IOBase* peripheral) {
    const auto& mmEntry = m_peripherals.at(peripheral);
    ProcessorHandler::get()->getMemory().removeRegion(mmEntry.startAddr, mmEntry.size);
}

IOBase* IOManager::createPeripheral(IOType type) {
    auto* peripheral = IOFactories.at(type)(nullptr);

    connect(peripheral, &IOBase::sizeChanged, [=] { this->peripheralSizeChanged(peripheral); });

    // Assign base address to peripheral
    const uint32_t base = nextPeripheralAddress();
    m_peripherals[peripheral] = {base, peripheral->size(), peripheral->name()};
    registerPeripheralWithProcessor(peripheral);
    refreshMemoryMap();

    return peripheral;
}

void IOManager::removePeripheral(IOBase* peripheral) {
    auto periphit = m_peripherals.find(peripheral);
    Q_ASSERT(periphit != m_peripherals.end());
    unregisterPeripheralWithProcessor(peripheral);
    m_peripherals.erase(periphit);
    refreshMemoryMap();
}

void IOManager::refreshAllPeriphsToProcessor() {
    for (const auto& periph : m_peripherals) {
        registerPeripheralWithProcessor(periph.first);
    }
}

void IOManager::refreshMemoryMap() {
    m_memoryMap.clear();

    for (const auto& periph : m_peripherals) {
        m_memoryMap[periph.second.startAddr] = periph.second;
    }

    const auto& program = ProcessorHandler::get()->getProgram();
    if (program) {
        for (const auto& section : program.get()->sections) {
            m_memoryMap[section.second.address] =
                MemoryMapEntry{static_cast<uint32_t>(section.second.address),
                               static_cast<unsigned>(section.second.data.size()), section.second.name};
        }
    }

    emit memoryMapChanged();
}

}  // namespace Ripes
