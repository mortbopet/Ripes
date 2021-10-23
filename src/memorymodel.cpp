#include "memorymodel.h"

#include <QBrush>
#include <QFont>

#include "fonts.h"
#include "processorhandler.h"

namespace Ripes {

MemoryModel::MemoryModel(QObject* parent) : QAbstractTableModel(parent) {}

int MemoryModel::columnCount(const QModelIndex&) const {
    return FIXED_COLUMNS_CNT + ProcessorHandler::currentISA()->bytes() /* byte columns */;
}

int MemoryModel::rowCount(const QModelIndex&) const {
    return m_rowsVisible;
}

void MemoryModel::processorWasClocked() {
    // Reload model
    beginResetModel();
    endResetModel();
}

AInt maxAddress() {
    return vsrtl::generateBitmask(ProcessorHandler::currentISA()->bits());
}

void MemoryModel::setCentralAddress(AInt address) {
    address = address - (address % ProcessorHandler::currentISA()->bytes());
    m_centralAddress = address;
    processorWasClocked();
}

// Checks whether an overflow or underflow error occurred when calculating the new address, relative to the current
// address
bool validAddressChange(AInt currentAddress, AInt newAddress) {
    bool validAddress = true;
    const AIntS signed_center = static_cast<AIntS>(currentAddress);
    const AIntS signed_aligned = static_cast<AIntS>(newAddress);

    // Arithmetic underflow and overflow check (overflow, if ISA bytes == sizeof(AIntS))
    validAddress &= (signed_center >= 0 && signed_aligned >= 0) || (signed_center < 0 && signed_aligned < 0);

    // Overflow check 2: if ISA bytes < sizeof(AIntS)
    validAddress &= newAddress < maxAddress();
    return validAddress;
}

void MemoryModel::offsetCentralAddress(int rowOffset) {
    const int byteOffset = rowOffset * ProcessorHandler::currentISA()->bytes();
    const AInt newCenterAddress = m_centralAddress + byteOffset;
    m_centralAddress = validAddressChange(m_centralAddress, newCenterAddress) ? newCenterAddress : m_centralAddress;
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

void MemoryModel::setRowsVisible(int rows) {
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
        return QFont(Fonts::monospace, 11);
    }

    const auto bytes = ProcessorHandler::currentISA()->bytes();
    // Calculate the word-aligned address corresponding to the row of the current index.
    // If the central address is at one of its two extrema, based on the address space of the processor, the aligned
    // address is invalid.
    /*
    AInt alignedAddress;
    bool validAddress = true;
    if (m_centralAddress == 0x0) {
        validAddress &= index.row() < m_rowsVisible / 2;
    }
    if (m_centralAddress >= maxAddress()) {
        validAddress &= index.row() > m_rowsVisible / 2;
    }
    if (validAddress) {
        alignedAddress = static_cast<AInt>(m_centralAddress) + ((((m_rowsVisible * bytes) / 2) / bytes) * bytes) -
                         (index.row() * bytes);
    }
*/

    const AInt alignedAddress =
        static_cast<AInt>(m_centralAddress) + ((((m_rowsVisible * bytes) / 2) / bytes) * bytes) - (index.row() * bytes);
    const bool validAddress = validAddressChange(m_centralAddress, alignedAddress);

    const unsigned byteOffset = index.column() - FIXED_COLUMNS_CNT;

    if (index.column() == Column::Address) {
        if (role == Qt::DisplayRole) {
            return addrData(alignedAddress, validAddress);
        } else if (role == Qt::ForegroundRole) {
            // Assign a brush if one of the byte-indexed address covered by the aligned address has been written to
            QVariant unusedAddressBrush;
            for (unsigned i = 0; i < ProcessorHandler::currentISA()->bytes(); ++i) {
                QVariant addressBrush = fgColorData(alignedAddress, i, validAddress);
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
                return fgColorData(alignedAddress, index.column() == Column::WordValue ? 0 : byteOffset, validAddress);
            case Qt::DisplayRole:
                if (index.column() == Column::WordValue) {
                    return wordData(alignedAddress, validAddress);
                } else {
                    return byteData(alignedAddress, byteOffset, validAddress);
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

QVariant MemoryModel::addrData(AInt address, bool validAddress) const {
    if (!validAddress) {
        return "-";
    }
    return encodeRadixValue(address, Radix::Hex, ProcessorHandler::currentISA()->bytes());
}

QVariant MemoryModel::fgColorData(AInt address, AInt byteOffset, bool validAddress) const {
    if (!validAddress || !ProcessorHandler::getMemory().contains(address + byteOffset)) {
        return QBrush(Qt::lightGray);
    } else {
        return QVariant();  // default
    }
}

QVariant MemoryModel::byteData(AInt address, AInt byteOffset, bool validAddress) const {
    if (!validAddress) {
        return "-";
    } else if (!ProcessorHandler::getMemory().contains(address + byteOffset)) {
        // Dont read the memory (this will create an entry in the memory if done so). Instead, create a "fake" entry in
        // the memory model, containing X's.
        return "X";
    } else {
        VInt value = ProcessorHandler::getMemory().readMemConst(address + byteOffset, 1);
        return encodeRadixValue(value & 0xFF, m_radix, 1);
    }
}

QVariant MemoryModel::wordData(AInt address, bool validAddress) const {
    if (!validAddress) {
        return "-";
    } else if (!ProcessorHandler::getMemory().contains(address)) {
        // Dont read the memory (this will create an entry in the memory if done so). Instead, create a "fake" entry in
        // the memory model, containing X's.
        return "X";
    } else {
        unsigned bytes = ProcessorHandler::currentISA()->bytes();
        return encodeRadixValue(ProcessorHandler::getMemory().readMemConst(address, bytes), m_radix, bytes);
    }
}

Qt::ItemFlags MemoryModel::flags(const QModelIndex&) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
}  // namespace Ripes
