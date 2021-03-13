#include "iomanager.h"

#include "processorhandler.h"
#include "ripessettings.h"

#include "VSRTL/core/vsrtl_sparsearray.h"

namespace Ripes {

IOManager::IOManager() : QObject(nullptr) {
    // Always re-register the currently active peripherals when the processor changes
    connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this, &IOManager::setProcessorPeripherals);
}

IOBase* IOManager::createPeripheral(IOType type) {
    auto* peripheral = IOFactories.at(type)(nullptr);

    // Assign base address to peripheral
    uint32_t base;
    if (m_memoryMap.empty()) {
        base = RipesSettings::value(RIPES_SETTING_PERIPHERALS_START).toUInt();
    } else {
        auto lastPeriph = m_memoryMap.rbegin();
        base = lastPeriph->first + lastPeriph->second.size;
    }

    ProcessorHandler::get()->getMemory().addRegion(
        base, peripheral->size(),
        vsrtl::core::IOFunctors{
            [peripheral](uint32_t offset, uint32_t value, uint32_t size) { peripheral->ioWrite(offset, value, size); },
            [peripheral](uint32_t offset, uint32_t size) { return peripheral->ioRead(offset, size); }});

    m_memoryMap[base] = {peripheral->size(), peripheral->name()};

    return peripheral;
}

void IOManager::setProcessorPeripherals() {}

}  // namespace Ripes
