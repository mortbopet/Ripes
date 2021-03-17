#include "iomanager.h"

#include "processorhandler.h"
#include "ripessettings.h"

#include "VSRTL/core/vsrtl_sparsearray.h"

namespace Ripes {

IOManager::IOManager() : QObject(nullptr) {
    // Always re-register the currently active peripherals when the processor changes
    connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this, &IOManager::setProcessorPeripherals);
    connect(ProcessorHandler::get(), &ProcessorHandler::reqProcessorReset, this, &IOManager::refreshMemoryMap);
    refreshMemoryMap();
}

IOBase* IOManager::createPeripheral(IOType type) {
    auto* peripheral = IOFactories.at(type)(nullptr);

    // Assign base address to peripheral
    uint32_t base;
    if (m_peripherals.empty()) {
        base = RipesSettings::value(RIPES_SETTING_PERIPHERALS_START).toUInt();
    } else {
        auto lastPeriph = m_peripherals.rbegin();
        base = lastPeriph->first + lastPeriph->second.size;
    }

    ProcessorHandler::get()->getMemory().addRegion(
        base, peripheral->size(),
        vsrtl::core::IOFunctors{
            [peripheral](uint32_t offset, uint32_t value, uint32_t size) { peripheral->ioWrite(offset, value, size); },
            [peripheral](uint32_t offset, uint32_t size) { return peripheral->ioRead(offset, size); }});

    m_peripherals[base] = {peripheral->size(), peripheral->name()};
    refreshMemoryMap();

    return peripheral;
}

void IOManager::setProcessorPeripherals() {}

void IOManager::refreshMemoryMap() {
    m_memoryMap = m_peripherals;

    const auto& program = ProcessorHandler::get()->getProgram();
    if (program) {
        for (const auto& section : program.get()->sections) {
            m_memoryMap[section.second.address] =
                MemoryMapEntry{static_cast<unsigned>(section.second.data.size()), section.second.name};
        }
    }

    emit memoryMapChanged();
}

}  // namespace Ripes
