#pragma once

#include <QAbstractTableModel>

#include "processorhandler.h"
#include "radix.h"

namespace Ripes {

class MemoryModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Address = 0 };
    MemoryModel(QObject* parent = nullptr);

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
    void setCentralAddress(uint32_t address);

private:
    QVariant addrData(long long address) const;
    QVariant byteData(long long address, unsigned byteOffset) const;
    QVariant fgColorData(long long address, unsigned byteOffset) const;

    Radix m_radix = Radix::Hex;

    long long m_centralAddress = 4;  // Address at the center of the model
    unsigned m_rowsVisible = 0;      // Number of rows currently visible in the view associated with the model
};
}  // namespace Ripes
