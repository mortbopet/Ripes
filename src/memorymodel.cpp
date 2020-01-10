#include "memorymodel.h"

NewMemoryModel::NewMemoryModel(ProcessorHandler& handler, QObject* parent)
    : QAbstractTableModel(parent), m_handler(handler) {}

int NewMemoryModel::columnCount(const QModelIndex&) const {
    return 1 /* address column */ + m_handler.getProcessor()->implementsISA().bytes() /* byte columns */;
}

int NewMemoryModel::rowCount(const QModelIndex&) const {
    return m_rowsVisible;
}

void NewMemoryModel::processorWasClocked() {
    // Reload model
    beginResetModel();
    endResetModel();
}

void NewMemoryModel::offsetCentralAddress(int rowOffset) {
    const int byteOffset = rowOffset * m_handler.getProcessor()->implementsISA().bytes();
    const long long newCenterAddress = static_cast<long long>(m_centralAddress) + byteOffset;
    m_centralAddress = newCenterAddress < 0 ? m_centralAddress : newCenterAddress;
    processorWasClocked();
}

QVariant NewMemoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == Column::Address) {
            return "Address";
        } else {
            return "+" + QString::number(section);
        }
    }
    return QVariant();
}

void NewMemoryModel::setRowsVisible(unsigned rows) {
    m_rowsVisible = rows;
    processorWasClocked();
}

QVariant NewMemoryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    const auto bytes = m_handler.getProcessor()->implementsISA().bytes();
    const long long alignedAddress = static_cast<long long>(m_centralAddress) +
                                     ((((m_rowsVisible * bytes) / 2) / bytes) * bytes) - (index.row() * bytes);
    const unsigned byteOffset = index.column() - 1;

    if (index.column() == Column::Address) {
        if (role == Qt::DisplayRole) {
            return addrData(alignedAddress);
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

void NewMemoryModel::setRadix(Radix r) {
    m_radix = r;
    processorWasClocked();
}

QVariant NewMemoryModel::addrData(long long address) const {
    if (address < 0) {
        return "-";
    }
    return encodeRadixValue(address, Radix::Hex);
}

QVariant NewMemoryModel::fgColorData(long long address, unsigned byteOffset) const {
    if (address < 0 || !m_handler.getProcessor()->getMemory().contains(address + byteOffset)) {
        return QBrush(Qt::lightGray);
    } else {
        return QVariant();  // default
    }
}

QVariant NewMemoryModel::byteData(long long address, unsigned byteOffset) const {
    if (address < 0) {
        return "-";
    } else if (!m_handler.getProcessor()->getMemory().contains(address + byteOffset)) {
        // Dont read the memory (this will create an entry in the memory if done so). Instead, create a "fake" entry in
        // the memory model, containing X's.
        return "X";
    } else {
        uint32_t value = m_handler.getProcessor()->getMemory().readMem(address);
        value = value >> (byteOffset * 8);
        return encodeRadixValue(value & 0xFF, m_radix, 8);
    }
}

Qt::ItemFlags NewMemoryModel::flags(const QModelIndex& index) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

MemoryModel::MemoryModel(MainMemory* memoryPtr, QObject* parent) : QStandardItemModel(parent) {
    m_memoryPtr = memoryPtr;

    updateModel();

    // Set headers
    setHeaderData(0, Qt::Horizontal, "Address");
    setHeaderData(1, Qt::Horizontal, "+0");
    setHeaderData(2, Qt::Horizontal, "+1");
    setHeaderData(3, Qt::Horizontal, "+2");
    setHeaderData(4, Qt::Horizontal, "+3");
}

void MemoryModel::setInvalidAddresLine(int row) {
    for (int i = 0; i < 5; i++) {
        auto invalidItem = new QStandardItem();
        invalidItem->setData("INVALID", Qt::DisplayRole);
        invalidItem->setTextAlignment(Qt::AlignCenter);
        setItem(row, i, invalidItem);
    };
}

void MemoryModel::offsetCentralAddress(int byteOffset) {
    // Changes the central address of the model by byteOffset, and updates the
    // model
    if (byteOffset % 4 == 0 && (m_centralAddress + byteOffset >= 0) && (m_centralAddress + byteOffset <= 0xffffffff)) {
        m_centralAddress += byteOffset;
        updateModel();
    }
}

void MemoryModel::updateModel() {
    for (int i = 0; i < m_addressRadius * 2 + 1; i++) {
        auto lineAddress = m_centralAddress + (m_addressRadius * 4) - i * 4;
        if (lineAddress < 0 || lineAddress > 0xffffffff) {
            // Memory is not available on negative addresses, set invalid data
            setInvalidAddresLine(i);
        } else {
            auto iter = m_memoryPtr->find(lineAddress);
            if (iter != m_memoryPtr->end()) {
                auto addr = new QStandardItem();
                addr->setData(QString("0x%1").arg(QString().setNum(iter->first, 16).rightJustified(8, '0')),
                              Qt::DisplayRole);
                addr->setTextAlignment(Qt::AlignCenter);
                auto b1 = new QStandardItem();
                b1->setData(iter->second, Qt::DisplayRole);
                b1->setTextAlignment(Qt::AlignCenter);
                setItem(i, 0, addr);
                setItem(i, 1, b1);
            } else {
                // No address in memory - create "fake" address in model
                auto addr = new QStandardItem();
                addr->setData(QString("0x%1").arg(QString().setNum(lineAddress, 16).rightJustified(8, '0')),
                              Qt::DisplayRole);
                addr->setTextAlignment(Qt::AlignCenter);
                auto b1 = new QStandardItem();
                b1->setData(0, Qt::DisplayRole);
                b1->setTextAlignment(Qt::AlignCenter);
                setItem(i, 0, addr);
                setItem(i, 1, b1);
            }
            for (int j = 1; j < 4; j++) {
                // Locate the remaining 3 bytes in the word
                auto byte = new QStandardItem();
                if ((iter = m_memoryPtr->find(lineAddress + j)) != m_memoryPtr->end()) {
                    byte->setData(iter->second, Qt::DisplayRole);
                    byte->setTextAlignment(Qt::AlignCenter);
                    setItem(i, j + 1, byte);
                } else {
                    // Memory is not initialized, set model data value to 0
                    byte->setData(0, Qt::DisplayRole);
                    byte->setTextAlignment(Qt::AlignCenter);
                    setItem(i, j + 1, byte);
                }
            }
        }
    }
}

void MemoryModel::jumpToAddress(uint32_t address) {
    m_centralAddress = address;
    updateModel();
}
