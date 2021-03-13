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

private:
    void setProcessorPeripherals();
    MemoryMap m_memoryMap;
};

}  // namespace Ripes
