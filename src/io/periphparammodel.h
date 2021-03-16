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
 * @brief The PeriphParamModel class
 * A model for Qt's MVC framework, responsible for representing and editing the parameters which are exposed by a memory
 * mapped peripheral.
 */
class PeriphParamModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Name, Value, NColumns };
    PeriphParamModel(IOBase* peripheral, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    IOBase* m_peripheral = nullptr;
};
}  // namespace Ripes
