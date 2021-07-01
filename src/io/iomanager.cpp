#include "iomanager.h"

#include "processorhandler.h"
#include "ripessettings.h"

#include <memory>
#include <ostream>

#include "VSRTL/core/vsrtl_addressspace.h"

namespace Ripes {

IOManager::IOManager() : QObject(nullptr) {
    // Always re-register the currently active peripherals when the processor changes
    connect(ProcessorHandler::get(), &ProcessorHandler::processorChanged, this,
            &IOManager::refreshAllPeriphsToProcessor);
    connect(ProcessorHandler::get(), &ProcessorHandler::processorReset, this, &IOManager::refreshMemoryMap);

    refreshMemoryMap();
}

QString IOManager::cSymbolsHeaderpath() const {
    if (m_symbolsHeaderFile) {
        return m_symbolsHeaderFile->fileName();
    } else {
        return QString();
    }
}

AInt IOManager::nextPeripheralAddress() const {
    AInt base = 0;
    if (m_periphMMappings.empty()) {
        base = static_cast<unsigned>(RipesSettings::value(RIPES_SETTING_PERIPHERALS_START).toInt());
    } else {
        for (const auto& periph : m_periphMMappings) {
            if (periph.second.end() > base) {
                base = periph.second.end();
            }
        }
    }
    return base;
}

AInt IOManager::assignBaseAddress(IOBase* peripheral) {
    unregisterPeripheralWithProcessor(peripheral);
    const AInt base = nextPeripheralAddress();
    m_periphMMappings[peripheral] = {base, peripheral->byteSize(), peripheral->name()};
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

void IOManager::peripheralSizeChanged(IOBase*) {
    assignBaseAddresses();
    refreshMemoryMap();
}

void IOManager::registerPeripheralWithProcessor(IOBase* peripheral) {
    ProcessorHandler::getMemory().addIORegion(
        m_periphMMappings.at(peripheral).startAddr, peripheral->byteSize(),
        vsrtl::core::IOFunctors{
            [peripheral](AInt offset, VInt value, unsigned size) { peripheral->ioWrite(offset, value, size); },
            [peripheral](AInt offset, unsigned size) { return peripheral->ioRead(offset, size); }});

    peripheral->memWrite = [](AInt address, VInt value, unsigned size) {
        ProcessorHandler::getMemory().writeMem(address, value, size);
    };
    peripheral->memRead = [](AInt address, unsigned size) {
        return ProcessorHandler::getMemory().readMem(address, size);
    };
}

void IOManager::unregisterPeripheralWithProcessor(IOBase* peripheral) {
    const auto& mmEntry = m_periphMMappings.find(peripheral);
    if (mmEntry != m_periphMMappings.end()) {
        ProcessorHandler::getMemory().removeIORegion(mmEntry->second.startAddr, mmEntry->second.size);
        m_periphMMappings.erase(mmEntry);
    }
}

IOBase* IOManager::createPeripheral(IOType type, unsigned forcedId) {
    auto* peripheral = IOFactories.at(type)(nullptr);

    connect(peripheral, &IOBase::sizeChanged, this, [=] { this->peripheralSizeChanged(peripheral); });
    connect(peripheral, &IOBase::aboutToDelete, this,
            [=](std::atomic<bool>& ok) { this->removePeripheral(peripheral, ok); });

    if (forcedId != UINT_MAX) {
        peripheral->setID(forcedId);
    }
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

    const auto& program = ProcessorHandler::getProgram();
    if (program) {
        for (const auto& section : program.get()->sections) {
            m_memoryMap[section.second.address] = MemoryMapEntry{
                section.second.address, static_cast<unsigned>(section.second.data.size()), section.second.name};
        }
    }

    updateSymbols();
    emit memoryMapChanged();
}

std::vector<std::pair<Symbol, AInt>> IOManager::assemblerSymbolsForPeriph(IOBase* peripheral) const {
    const QString& periphName = cName(peripheral->name());
    std::vector<std::pair<Symbol, AInt>> symbols;
    const auto& periphInfo = m_periphMMappings.at(peripheral);
    symbols.push_back({{periphName + "_BASE", Symbol::Type::Address}, periphInfo.startAddr});
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
    headerfile << "#ifndef RIPES_IO_HEADER";
    headerfile << "#define RIPES_IO_HEADER";
    for (const auto& p : m_periphMMappings) {
        const QString& periphName = cName(p.first->name());
        headerfile << "// *****************************************************************************";
        headerfile << "// * " + periphName;
        headerfile << "// *****************************************************************************";

        auto symbols = assemblerSymbolsForPeriph(p.first);
        m_assemblerSymbols.insert(symbols.begin(), symbols.end());

        for (const auto& symbol : assemblerSymbolsForPeriph(p.first)) {
            headerfile << "#define " + symbol.first.v + "\t" + "(0x" + QString::number(symbol.second, 16) + ")";
        }

        headerfile << "\n";
    }
    headerfile << "#endif // RIPES_IO_HEADER";

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
