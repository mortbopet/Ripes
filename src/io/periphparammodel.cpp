#include "periphparammodel.h"

#include <QHeaderView>

#include "iobase.h"
#include "processorhandler.h"

namespace Ripes {

PeriphParamModel::PeriphParamModel(IOBase* peripheral, QObject* parent)
    : QAbstractTableModel(parent), m_peripheral(peripheral) {}

int PeriphParamModel::columnCount(const QModelIndex&) const {
    return NColumns;
}

int PeriphParamModel::rowCount(const QModelIndex&) const {
    return m_peripheral->parameters().size();
}

QVariant PeriphParamModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Name:
                return "Name";
            case Column::Value:
                return "Value";
            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool PeriphParamModel::setData(const QModelIndex& index, const QVariant& value, int) {
    const unsigned idx = index.row();
    auto ret = m_peripheral->setParameter(idx, value);
    return ret == value;
}

QVariant PeriphParamModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    const unsigned idx = index.row();
    switch (index.column()) {
        case Column::Name: {
            if (role == Qt::DisplayRole) {
                return m_peripheral->parameters().at(idx).name;
            }
            break;
        }
        case Column::Value: {
            switch (role) {
                case Qt::DisplayRole:
                    return m_peripheral->parameters().at(idx).value;
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

Qt::ItemFlags PeriphParamModel::flags(const QModelIndex& index) const {
    const unsigned col = index.column();
    Qt::ItemFlags flags = Qt::ItemIsEnabled;

    if (col == Value) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}
}  // namespace Ripes
