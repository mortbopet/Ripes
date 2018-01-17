#include "memorymodel.h"

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
