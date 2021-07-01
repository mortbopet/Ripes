#pragma once

#include <QAbstractTableModel>

#include "radix.h"

namespace Ripes {

class MemoryModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Address = 0, WordValue = 1, FIXED_COLUMNS_CNT };
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
    void setRowsVisible(int rows);
    void offsetCentralAddress(int rowOffset);
    void setCentralAddress(Ripes::AInt address);

private:
    QVariant addrData(AInt address, bool validAddress) const;
    QVariant byteData(AInt address, AInt byteOffset, bool validAddress) const;
    QVariant wordData(AInt address, bool validAddress) const;
    QVariant fgColorData(AInt address, AInt byteOffset, bool validAddress) const;

    Radix m_radix = Radix::Hex;

    AInt m_centralAddress = 0;  // Memory address at the center of the model
    int m_rowsVisible = 0;      // Number of rows currently visible in the view associated with the model
};
}  // namespace Ripes
