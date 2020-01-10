#ifndef MEMORYMODEL_H
#define MEMORYMODEL_H

#include "mainmemory.h"

#include <QAbstractTableModel>
#include <QStandardItemModel>

#include <unordered_map>

#include "processorhandler.h"
#include "radix.h"

class NewMemoryModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Address = 0 };
    NewMemoryModel(ProcessorHandler& handler, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setRadix(Radix r);
    Radix getRadix() const { return m_radix; }

public slots:
    void processorWasClocked();
    void setRowsVisible(unsigned rows);
    void offsetCentralAddress(int rowOffset);

private:
    QVariant addrData(long long address) const;
    QVariant byteData(long long address, unsigned byteOffset) const;
    QVariant fgColorData(long long address, unsigned byteOffset) const;
    ProcessorHandler& m_handler;

    Radix m_radix = Radix::Unsigned;

    long long m_centralAddress = 4;  // Address at the center of the model
    unsigned m_rowsVisible = 0;      // Number of rows currently visible in the view associated with the model
};

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
