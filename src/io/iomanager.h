#pragma once

#include "iobase.h"
#include "ioregistry.h"

#include <QFile>

namespace Ripes {

struct MemoryMapEntry {
    uint32_t startAddr;
    unsigned size;
    QString name;
    uint32_t end() const { return startAddr + size; }
};

using MemoryMap = std::map<uint32_t, MemoryMapEntry>;

class IOManager : public QObject {
    Q_OBJECT

public:
    static IOManager& get() {
        static IOManager manager;
        return manager;
    }

    IOBase* createPeripheral(IOType type);
    void removePeripheral(IOBase* peripheral, std::atomic<bool>& ok);
    const MemoryMap& memoryMap() const { return m_memoryMap; }

    /**
     * @brief cSymbolsHeaderpath
     * @returns the path of a header file of #define's containing the current peripherals base addresses + memory mapped
     * registers
     */
    QString cSymbolsHeaderpath() const;

    /**
     * @brief assemblerSymbols
     * @returns as cSymbols, but as a map which can be directly loaded into the assembler.
     */
    std::map<QString, uint32_t> assemblerSymbols() const { return m_assemblerSymbols; }
    std::vector<std::pair<QString, uint32_t>> assemblerSymbolsForPeriph(IOBase* peripheral) const;

signals:
    void memoryMapChanged();
    void peripheralRemoved(QObject* peripheral);

private:
    IOManager();

    /**
     * @brief updateSymbols
     * Updates the set of exported memory-mapped symbols for the currently instantiated peripherals, as well as the
     * generated header file.
     */
    void updateSymbols();

    void refreshMemoryMap();
    void peripheralSizeChanged(IOBase* peripheral);

    /**
     * @brief registerPeripheralWithProcessor
     * Registers @param peripheral with the processor. Specifically, the peripheral hooks into the memory of the
     * processor, and creates the link between the peripheral memory read/write functionality, and the processor memory.
     */
    void registerPeripheralWithProcessor(IOBase* peripheral);
    void unregisterPeripheralWithProcessor(IOBase* peripheral);

    /**
     * @brief refreshAllPeriphsToProcessor
     * Shall be called after changing the processor. Registers all the currently initialized peripherals to the (new)
     * processor.
     */
    void refreshAllPeriphsToProcessor();

    /**
     * @brief nextPeripheralAddress
     * @returns a valid base address for a new peripheral
     */
    uint32_t nextPeripheralAddress() const;

    MemoryMap m_memoryMap;
    std::map<IOBase*, MemoryMapEntry> m_peripherals;
    std::map<QString, uint32_t> m_assemblerSymbols;
    std::unique_ptr<QFile> m_symbolsHeaderFile;
};

}  // namespace Ripes
