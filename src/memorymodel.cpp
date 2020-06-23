#include "memorymodel.h"

#include <QBrush>
#include <QFont>

namespace Ripes {

MemoryModel::MemoryModel(QObject* parent) : QAbstractTableModel(parent) {}

int MemoryModel::columnCount(const QModelIndex&) const {
    return FIXED_COLUMNS_CNT + ProcessorHandler::get()->currentISA()->bytes() /* byte columns */;
}

int MemoryModel::rowCount(const QModelIndex&) const {
    return m_rowsVisible;
}

void MemoryModel::processorWasClocked() {
    // Reload model
    beginResetModel();
    endResetModel();
}

bool MemoryModel::validAddress(long long address) const {
    return !(address < 0 || (address > (std::pow(2, ProcessorHandler::get()->currentISA()->bits()) - 1)));
}

void MemoryModel::setCentralAddress(uint32_t address) {
    address = address - (address % ProcessorHandler::get()->currentISA()->bytes());
    m_centralAddress = address;
    processorWasClocked();
}

void MemoryModel::offsetCentralAddress(int rowOffset) {
    const int byteOffset = rowOffset * ProcessorHandler::get()->currentISA()->bytes();
    const long long newCenterAddress = static_cast<long long>(m_centralAddress) + byteOffset;
    m_centralAddress = !validAddress(newCenterAddress) ? m_centralAddress : newCenterAddress;
    processorWasClocked();
}

QVariant MemoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Address:
                return "Address";
            case Column::WordValue:
                return "Word";
            default:
                return "Byte " + QString::number(section - FIXED_COLUMNS_CNT);
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

    if (role == Qt::FontRole) {
        return QFont("Inconsolata", 11);
    }

    const auto bytes = ProcessorHandler::get()->currentISA()->bytes();
    const long long alignedAddress = static_cast<long long>(m_centralAddress) +
                                     ((((m_rowsVisible * bytes) / 2) / bytes) * bytes) - (index.row() * bytes);
    const unsigned byteOffset = index.column() - FIXED_COLUMNS_CNT;

    if (index.column() == Column::Address) {
        if (role == Qt::DisplayRole) {
            return addrData(alignedAddress);
        } else if (role == Qt::ForegroundRole) {
            // Assign a brush if one of the byte-indexed address covered by the aligned address has been written to
            QVariant unusedAddressBrush;
            for (unsigned i = 0; i < ProcessorHandler::get()->currentISA()->bytes(); i++) {
                QVariant addressBrush = fgColorData(alignedAddress, i);
                if (addressBrush.isNull()) {
                    return addressBrush;
                } else {
                    unusedAddressBrush = addressBrush;
                }
            }
            return unusedAddressBrush;
        }
    } else {
        switch (role) {
            case Qt::ForegroundRole:
                return fgColorData(alignedAddress, index.column() == Column::WordValue ? 0 : byteOffset);
            case Qt::DisplayRole:
                if (index.column() == Column::WordValue) {
                    return wordData(alignedAddress);
                } else {
                    return byteData(alignedAddress, byteOffset);
                }
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
    if (!validAddress(address)) {
        return "-";
    }
    return encodeRadixValue(address, Radix::Hex);
}

QVariant MemoryModel::fgColorData(long long address, unsigned byteOffset) const {
    if (!validAddress(address) ||
        !ProcessorHandler::get()->getMemory().contains(static_cast<unsigned>(address + byteOffset))) {
        return QBrush(Qt::lightGray);
    } else {
        return QVariant();  // default
    }
}

QVariant MemoryModel::byteData(long long address, unsigned byteOffset) const {
    if (!validAddress(address)) {
        return "-";
    } else if (!ProcessorHandler::get()->getMemory().contains(static_cast<unsigned>(address + byteOffset))) {
        // Dont read the memory (this will create an entry in the memory if done so). Instead, create a "fake" entry in
        // the memory model, containing X's.
        return "X";
    } else {
        uint32_t value = ProcessorHandler::get()->getMemory().readValue<uint32_t>(static_cast<uint32_t>(address));
        value = value >> (byteOffset * 8);
        return encodeRadixValue(value & 0xFF, m_radix, 8);
    }
}

QVariant MemoryModel::wordData(long long address) const {
    if (!validAddress(address)) {
        return "-";
    } else if (!ProcessorHandler::get()->getMemory().contains(static_cast<unsigned>(address))) {
        // Dont read the memory (this will create an entry in the memory if done so). Instead, create a "fake" entry in
        // the memory model, containing X's.
        return "X";
    } else {
        uint32_t value = ProcessorHandler::get()->getMemory().readValue<uint32_t>(static_cast<uint32_t>(address));
        return encodeRadixValue(value, m_radix, ProcessorHandler::get()->currentISA()->bits());
    }
}

Qt::ItemFlags MemoryModel::flags(const QModelIndex&) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
}  // namespace Ripes
