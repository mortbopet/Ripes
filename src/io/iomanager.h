#pragma once

#include "iobase.h"
#include "ioregistry.h"

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
    IOManager();
    IOBase* createPeripheral(IOType type);
    void removePeripheral(IOBase* peripheral);
    const MemoryMap& memoryMap() const { return m_memoryMap; }

signals:
    void memoryMapChanged();

private:
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
};

}  // namespace Ripes
