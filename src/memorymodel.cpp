#include "memorymodel.h"

#include <QBrush>
#include <QFont>

MemoryModel::MemoryModel(QObject* parent) : QAbstractTableModel(parent) {}

int MemoryModel::columnCount(const QModelIndex&) const {
    return 1 /* address column */ +
           ProcessorHandler::get()->getProcessor()->implementsISA()->bytes() /* byte columns */;
}

int MemoryModel::rowCount(const QModelIndex&) const {
    return m_rowsVisible;
}

void MemoryModel::processorWasClocked() {
    // Reload model
    beginResetModel();
    endResetModel();
}

void MemoryModel::setCentralAddress(uint32_t address) {
    address = address - (address % ProcessorHandler::get()->getProcessor()->implementsISA()->bytes());
    m_centralAddress = address;
    processorWasClocked();
}

void MemoryModel::offsetCentralAddress(int rowOffset) {
    const int byteOffset = rowOffset * ProcessorHandler::get()->getProcessor()->implementsISA()->bytes();
    const long long newCenterAddress = static_cast<long long>(m_centralAddress) + byteOffset;
    m_centralAddress = newCenterAddress < 0 ? m_centralAddress : newCenterAddress;
    processorWasClocked();
}

QVariant MemoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == Column::Address) {
            return "Address";
        } else {
            return "+" + QString::number(section);
        }
    }
    return QVariant();
}

void MemoryModel::setRowsVisible(unsigned rows) {
    m_rowsVisible = rows;
    processorWasClocked();
}

QVariant MemoryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    const auto bytes = ProcessorHandler::get()->getProcessor()->implementsISA()->bytes();
    const long long alignedAddress = static_cast<long long>(m_centralAddress) +
                                     ((((m_rowsVisible * bytes) / 2) / bytes) * bytes) - (index.row() * bytes);
    const unsigned byteOffset = index.column() - 1;

    if (index.column() == Column::Address) {
        if (role == Qt::DisplayRole) {
            return addrData(alignedAddress);
        } else if (role == Qt::ForegroundRole && alignedAddress < 0) {
            return fgColorData(alignedAddress, 0);
        }
    } else {
        switch (role) {
            case Qt::FontRole:
                return QFont("monospace");
            case Qt::ForegroundRole:
                return fgColorData(alignedAddress, byteOffset);
            case Qt::DisplayRole:
                return byteData(alignedAddress, byteOffset);
            default:
                break;
        }
    }

    return QVariant();
}

void MemoryModel::setRadix(Radix r) {
    m_radix = r;
    processorWasClocked();
}

QVariant MemoryModel::addrData(long long address) const {
    if (address < 0) {
        return "-";
    }
    return encodeRadixValue(address, Radix::Hex);
}

QVariant MemoryModel::fgColorData(long long address, unsigned byteOffset) const {
    if (address < 0 || !ProcessorHandler::get()->getMemory().contains(address + byteOffset)) {
        return QBrush(Qt::lightGray);
    } else {
        return QVariant();  // default
    }
}

QVariant MemoryModel::byteData(long long address, unsigned byteOffset) const {
    if (address < 0) {
        return "-";
    } else if (!ProcessorHandler::get()->getMemory().contains(address + byteOffset)) {
        // Dont read the memory (this will create an entry in the memory if done so). Instead, create a "fake" entry in
        // the memory model, containing X's.
        return "X";
    } else {
        uint32_t value = ProcessorHandler::get()->getMemory().readMemConst(address);
        value = value >> (byteOffset * 8);
        return encodeRadixValue(value & 0xFF, m_radix, 8);
    }
}

Qt::ItemFlags MemoryModel::flags(const QModelIndex& index) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
