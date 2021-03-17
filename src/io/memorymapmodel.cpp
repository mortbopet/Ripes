#include "memorymapmodel.h"

#include <QHeaderView>

#include "iobase.h"
#include "iomanager.h"
#include "processorhandler.h"
#include "radix.h"

namespace Ripes {

MemoryMapModel::MemoryMapModel(const IOManager* ioManager, QObject* parent)
    : QAbstractTableModel(parent), m_ioManager(ioManager) {
    connect(m_ioManager, &IOManager::memoryMapChanged, this, &MemoryMapModel::memoryMapChanged);
}

void MemoryMapModel::memoryMapChanged() {
    // Let's assume that this is not a hot function, and just update the whole view.
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
    emit layoutChanged();
}

int MemoryMapModel::columnCount(const QModelIndex&) const {
    return NColumns;
}

int MemoryMapModel::rowCount(const QModelIndex&) const {
    return m_ioManager->memoryMap().size();
}

QVariant MemoryMapModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Name:
                return "Name";
            case Column::AddressRange:
                return "Range";
            case Column::Size:
                return "Size";
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant MemoryMapModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    const unsigned idx = index.row();
    auto memoryMapEntry = std::next(m_ioManager->memoryMap().begin(), idx);
    switch (index.column()) {
        case Column::Name: {
            if (role == Qt::DisplayRole) {
                return memoryMapEntry->second.name;
            }
            break;
        }
        case Column::AddressRange: {
            switch (role) {
                case Qt::DisplayRole:
                    return encodeRadixValue(memoryMapEntry->first, Radix::Hex) + " - " +
                           encodeRadixValue(memoryMapEntry->first + memoryMapEntry->second.size, Radix::Hex);
                case Qt::FontRole:
                    return QFont("Inconsolata", 11);
                case Qt::TextAlignmentRole:
                    return Qt::AlignCenter;
            }
            break;
        }
        case Column::Size: {
            switch (role) {
                case Qt::DisplayRole: {
                    return QString::number(memoryMapEntry->second.size);
                }
                case Qt::TextAlignmentRole:
                    return Qt::AlignCenter;
            }
            break;
        }

        default:
            return QVariant();
    }
    return QVariant();
}

Qt::ItemFlags MemoryMapModel::flags(const QModelIndex&) const {
    return Qt::ItemIsEnabled;
}
}  // namespace Ripes
