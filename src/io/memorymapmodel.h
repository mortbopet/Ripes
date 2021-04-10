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
class IOManager;

/**
 * @brief The MemoryMapModel class
 * A model for Qt's MVC framework, responsible for representing the current state of the memory system of the simulator.
 */
class MemoryMapModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Name, Size, AddressRange, NColumns };
    MemoryMapModel(const IOManager* iomanager, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

public slots:
    void memoryMapChanged();

private:
    IOManager const* m_ioManager = nullptr;
};
}  // namespace Ripes
