#ifndef MEMORYMODEL_H
#define MEMORYMODEL_H

#include "mainmemory.h"

#include <QStandardItemModel>

#include <unordered_map>

class MemoryModel : public QStandardItemModel {
    Q_OBJECT

public:
    explicit MemoryModel(MainMemory* memoryPtr, QObject* parent = nullptr);

    // Custom functionality
    void offsetCentralAddress(int byteOffset);
    void setAddressCount(int count);
    long long getCentralAddress() const { return m_centralAddress; }
    int getModelRows() const { return rowCount(); }

    void updateModel();
public slots:
    void jumpToAddress(uint32_t address);

private:
    void setInvalidAddresLine(int row);

    // instead of direct access to the memory, we should have a pointer to the runner, which can return requested
    // values, or null, if memory doesnt exist
    MainMemory* m_memoryPtr;

    long long m_centralAddress = 4;  // Address at the center of the model
    int m_addressRadius = 20;        // amount of addresses in each direction, from
                                     // centralAddress
};

#endif  // MEMORYMODEL_H
