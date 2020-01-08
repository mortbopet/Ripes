#include "registermodel.h"

#include <QHeaderView>

using namespace vsrtl;

RegisterModel::RegisterModel(ProcessorHandler& handler, QObject* parent)
    : QAbstractTableModel(parent), m_handler(handler) {}

int RegisterModel::columnCount(const QModelIndex&) const {
    return NColumns;
}

int RegisterModel::rowCount(const QModelIndex&) const {
    return 32;
}

void RegisterModel::processorWasClocked() {
    // Reload model
    beginResetModel();
    endResetModel();
}

bool RegisterModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    const int i = index.row();
    if (index.column() == Column::Value) {
        m_handler.setRegisterValue(i, value.toUInt());
    }
    return false;
}

QVariant RegisterModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Name:
                return "Name";
            case Column::Alias:
                return "Alias";
            case Column::Value:
                return "Value";
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant RegisterModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    const unsigned idx = index.row();
    if (role == Qt::ToolTipRole) {
        return tooltipData(idx);
    }

    switch (index.column()) {
        case Column::Name: {
            if (role == Qt::DisplayRole) {
                return nameData(idx);
            }
            break;
        }
        case Column::Alias: {
            if (role == Qt::DisplayRole) {
                return aliasData(idx);
            }
            break;
        }
        case Column::Value: {
            switch (role) {
                case Qt::DisplayRole:
                    return valueData(idx);
                case Qt::UserRole:
                    return QVariant::fromValue(m_radix);
                case Qt::FontRole:
                    return QFont("monospace");
                case Qt::ForegroundRole:
                    return QBrush(Qt::blue);
                default:
                    break;
            }
        }
    }
    return QVariant();
}

void RegisterModel::setRadix(Radix r) {
    m_radix = r;
    processorWasClocked();
}

QVariant RegisterModel::nameData(unsigned idx) const {
    return m_handler.getProcessor()->implementsISA().regName(idx);
}

QVariant RegisterModel::aliasData(unsigned idx) const {
    return m_handler.getProcessor()->implementsISA().regAlias(idx);
}

QVariant RegisterModel::tooltipData(unsigned idx) const {
    return m_handler.getProcessor()->implementsISA().regInfo(idx);
}

QVariant RegisterModel::valueData(unsigned idx) const {
    return encodeRadixValue(m_handler.getRegisterValue(idx), m_radix);
}

Qt::ItemFlags RegisterModel::flags(const QModelIndex& index) const {
    const auto def =
        m_handler.getProcessor()->implementsISA().regIsReadOnly(index.row()) ? Qt::NoItemFlags : Qt::ItemIsEnabled;
    if (index.column() == Column::Value)
        return Qt::ItemIsEditable | def;
    return def;
}
