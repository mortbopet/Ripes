#include "registermodel.h"

#include <QHeaderView>

#include "processorhandler.h"

namespace Ripes {

using namespace vsrtl;

RegisterModel::RegisterModel(RegisterFileType rft, QObject* parent) : m_rft(rft), QAbstractTableModel(parent) {}

std::vector<uint32_t> RegisterModel::gatherRegisterValues() {
    std::vector<uint32_t> vals;
    for (int i = 0; i < rowCount(); i++) {
        vals.push_back(ProcessorHandler::get()->getRegisterValue(m_rft, i));
    }
    return vals;
}

int RegisterModel::columnCount(const QModelIndex&) const {
    return NColumns;
}

int RegisterModel::rowCount(const QModelIndex&) const {
    return ProcessorHandler::get()->currentISA()->regCnt();
}

void RegisterModel::processorWasClocked() {
    // Reload model
    beginResetModel();
    endResetModel();
    const auto newRegValues = gatherRegisterValues();
    if (m_regValues.size() != 0) {
        for (unsigned i = 0; i < newRegValues.size(); i++) {
            if (m_regValues[i] != newRegValues[i]) {
                m_mostRecentlyModifiedReg = i;
                emit registerChanged(i);
                break;
            }
        }
    }
    m_regValues = newRegValues;
}

bool RegisterModel::setData(const QModelIndex& index, const QVariant& value, int) {
    const int i = index.row();
    if (index.column() == Column::Value) {
        bool ok;
        uint32_t v = decodeRadixValue(value.toString(), m_radix, &ok);
        if (ok) {
            ProcessorHandler::get()->setRegisterValue(m_rft, i, v);
            emit dataChanged(index, index);
            return true;
        }
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
    } else if (role == Qt::BackgroundRole && index.row() == m_mostRecentlyModifiedReg) {
        return QBrush(QColor("#FDB515"));
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
                    return QFont("Inconsolata", 11);
                case Qt::ForegroundRole:
                    return QBrush(Qt::blue);
                default:
                    break;
            }
        }
    }
    return QVariant();
}

void RegisterModel::setRadix(Ripes::Radix r) {
    m_radix = r;
    processorWasClocked();
}

QVariant RegisterModel::nameData(unsigned idx) const {
    return ProcessorHandler::get()->currentISA()->regName(idx);
}

QVariant RegisterModel::aliasData(unsigned idx) const {
    return ProcessorHandler::get()->currentISA()->regAlias(idx);
}

QVariant RegisterModel::tooltipData(unsigned idx) const {
    return ProcessorHandler::get()->currentISA()->regInfo(idx);
}

QVariant RegisterModel::valueData(unsigned idx) const {
    return encodeRadixValue(ProcessorHandler::get()->getRegisterValue(m_rft, idx), m_radix);
}

Qt::ItemFlags RegisterModel::flags(const QModelIndex& index) const {
    const auto def =
        ProcessorHandler::get()->currentISA()->regIsReadOnly(index.row()) ? Qt::NoItemFlags : Qt::ItemIsEnabled;
    if (index.column() == Column::Value)
        return Qt::ItemIsEditable | def;
    return def;
}
}  // namespace Ripes
