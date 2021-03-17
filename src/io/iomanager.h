#pragma once

#include "iobase.h"
#include "ioregistry.h"

namespace Ripes {

struct MemoryMapEntry {
    unsigned size;
    QString name;
};

using MemoryMap = std::map<uint32_t, MemoryMapEntry>;

class IOManager : public QObject {
    Q_OBJECT

public:
    IOManager();
    IOBase* createPeripheral(IOType type);
    const MemoryMap& memoryMap() const { return m_memoryMap; }

signals:
    void memoryMapChanged();

private:
    void refreshMemoryMap();
    void setProcessorPeripherals();
    MemoryMap m_memoryMap;
    MemoryMap m_peripherals;
};

}  // namespace Ripes
