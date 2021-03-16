#pragma once

#include <QColor>
#include <set>
#include "defines.h"
#include "mainmemory.h"

#include "isa/isainfo.h"
#include "radix.h"

#include <QAbstractTableModel>

namespace Ripes {
class IOBase;

/**
 * @brief The RegisterMapModel class
 * A model for Qt's MVC framework, responsible for representing the register map exposed by a memory mapped peripheral.
 * The model is read-only.
 */
class RegisterMapModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Name, Address, RW, BitWidth, NColumns };
    RegisterMapModel(IOBase* peripheral, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

public slots:
    void regMapChanged();

private:
    IOBase* m_peripheral = nullptr;
};
}  // namespace Ripes
