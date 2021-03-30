#include "iomanager.h"

#include "processorhandler.h"
#include "ripessettings.h"

#include <memory>
#include <ostream>

#include "VSRTL/core/vsrtl_sparsearray.h"

namespace Ripes {

IOManager::IOManager() : QObject(nullptr) {
    // Always re-register the currently active peripherals when the processor changes
    connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
            &IOManager::refreshAllPeriphsToProcessor);
    connect(ProcessorHandler::get(), &ProcessorHandler::reqProcessorReset, this, &IOManager::refreshMemoryMap);

    refreshMemoryMap();
}

QString IOManager::cSymbolsHeaderpath() const {
    if (m_symbolsHeaderFile) {
        return m_symbolsHeaderFile->fileName();
    } else {
        return QString();
    }
}

uint32_t IOManager::nextPeripheralAddress() const {
    uint32_t base = 0;
    if (m_periphMMappings.empty()) {
        base = RipesSettings::value(RIPES_SETTING_PERIPHERALS_START).toUInt();
    } else {
        for (const auto& periph : m_periphMMappings) {
            if (periph.second.end() > base) {
                base = periph.second.end();
            }
        }
    }
    return base;
}

uint32_t IOManager::assignBaseAddress(IOBase* peripheral) {
    unregisterPeripheralWithProcessor(peripheral);
    const uint32_t base = nextPeripheralAddress();
    m_periphMMappings[peripheral] = {base, peripheral->size(), peripheral->name()};
    registerPeripheralWithProcessor(peripheral);
    return base;
}

void IOManager::assignBaseAddresses() {
    // First unassign all base addresses to start with a clean address map
    for (const auto& periph : m_peripherals) {
        unregisterPeripheralWithProcessor(periph);
    }
    for (const auto& periph : m_peripherals) {
        assignBaseAddress(periph);
    }
    refreshMemoryMap();
}

void IOManager::peripheralSizeChanged(IOBase* peripheral) {
    assignBaseAddresses();
    refreshMemoryMap();
}

void IOManager::registerPeripheralWithProcessor(IOBase* peripheral) {
    ProcessorHandler::get()->getMemory().addRegion(
        m_periphMMappings.at(peripheral).startAddr, peripheral->size(),
        vsrtl::core::IOFunctors{
            [peripheral](uint32_t offset, uint32_t value, uint32_t size) { peripheral->ioWrite(offset, value, size); },
            [peripheral](uint32_t offset, uint32_t size) { return peripheral->ioRead(offset, size); }});

    peripheral->memWrite = [](uint32_t address, uint32_t value, uint32_t size) {
        ProcessorHandler::get()->getMemory().writeMem(address, value, size);
    };
    peripheral->memRead = [](uint32_t address, uint32_t size) {
        return ProcessorHandler::get()->getMemory().readMem(address, size);
    };
}

void IOManager::unregisterPeripheralWithProcessor(IOBase* peripheral) {
    const auto& mmEntry = m_periphMMappings.find(peripheral);
    if (mmEntry != m_periphMMappings.end()) {
        ProcessorHandler::get()->getMemory().removeRegion(mmEntry->second.startAddr, mmEntry->second.size);
        m_periphMMappings.erase(mmEntry);
    }
}

IOBase* IOManager::createPeripheral(IOType type) {
    auto* peripheral = IOFactories.at(type)(nullptr);

    connect(peripheral, &IOBase::sizeChanged, [=] { this->peripheralSizeChanged(peripheral); });
    connect(peripheral, &IOBase::aboutToDelete, [=](std::atomic<bool>& ok) { this->removePeripheral(peripheral, ok); });

    m_peripherals.insert(peripheral);
    assignBaseAddress(peripheral);
    refreshMemoryMap();

    return peripheral;
}

void IOManager::removePeripheral(IOBase* peripheral, std::atomic<bool>& ok) {
    auto periphit = m_peripherals.find(peripheral);
    Q_ASSERT(periphit != m_peripherals.end());
    unregisterPeripheralWithProcessor(peripheral);
    m_peripherals.erase(periphit);

    emit peripheralRemoved(peripheral);
    refreshMemoryMap();

    ok = true;
}

void IOManager::refreshAllPeriphsToProcessor() {
    for (const auto& periph : m_periphMMappings) {
        registerPeripheralWithProcessor(periph.first);
    }
}

void IOManager::refreshMemoryMap() {
    m_memoryMap.clear();

    for (const auto& periph : m_periphMMappings) {
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
    updateSymbols();
}

std::vector<std::pair<QString, uint32_t>> IOManager::assemblerSymbolsForPeriph(IOBase* peripheral) const {
    const QString& periphName = cName(peripheral->name());
    std::vector<std::pair<QString, uint32_t>> symbols;
    const auto& periphInfo = m_periphMMappings.at(peripheral);
    symbols.push_back({periphName + "_BASE", periphInfo.startAddr});
    symbols.push_back({periphName + "_SIZE", periphInfo.size});

    for (const auto& reg : peripheral->registers()) {
        if (reg.exported) {
            const QString base = periphName + "_" + cName(reg.name);
            const QString offset = base + "_OFFSET";
            symbols.push_back({offset, reg.address});
            symbols.push_back({base, reg.address + periphInfo.startAddr});
        }
    }

    if (auto* extraSymbols = peripheral->extraSymbols()) {
        for (const auto& extraSymbol : *extraSymbols) {
            symbols.push_back({periphName + "_" + cName(extraSymbol.name), extraSymbol.value});
        }
    }

    return symbols;
}

void IOManager::updateSymbols() {
    m_assemblerSymbols.clear();

    // Generate symbol mapping + header file
    QStringList headerfile;
    headerfile << "#pragma once";
    for (const auto& p : m_periphMMappings) {
        const QString& periphName = cName(p.first->name());
        headerfile << "// *****************************************************************************";
        headerfile << "// * " + periphName;
        headerfile << "// *****************************************************************************";

        auto symbols = assemblerSymbolsForPeriph(p.first);
        m_assemblerSymbols.insert(symbols.begin(), symbols.end());

        for (const auto& symbol : assemblerSymbolsForPeriph(p.first)) {
            headerfile << "#define " + symbol.first + "\t" + "(0x" + QString::number(symbol.second, 16) + ")";
        }

        headerfile << "\n";
    }

    // Store header file at a temporary location
    if (!(m_symbolsHeaderFile && (QFile::exists(m_symbolsHeaderFile->fileName())))) {
        m_symbolsHeaderFile = std::make_unique<QFile>(QDir::tempPath() + QDir::separator() + "ripes_system.h");
    }

    if (m_symbolsHeaderFile->open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        m_symbolsHeaderFile->write(headerfile.join('\n').toUtf8());
        m_symbolsHeaderFile->close();
    }
}

}  // namespace Ripes
