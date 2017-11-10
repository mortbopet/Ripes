#include "memorymodel.h"

MemoryModel::MemoryModel(memory *memoryPtr, QObject *parent)
    : QStandardItemModel(parent) {
  m_memoryPtr = memoryPtr;

  // Create initial view
  // i is a byte index
  for (int i = 0; i < m_addressRadius * 2 + 1; i++) {
    auto lineAddress = m_centralAddress + (m_addressRadius * 4) - i * 4;
    auto iter = m_memoryPtr->find(lineAddress);
    if (iter != m_memoryPtr->end()) {
      auto addr = new QStandardItem();
      addr->setData(
          QString("0x%1").arg(
              QString().setNum(iter->first, 16).rightJustified(8, '0')),
          Qt::DisplayRole);
      auto b1 = new QStandardItem();
      b1->setData(iter->second, Qt::DisplayRole);
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

  // Set headers
  setHeaderData(0, Qt::Horizontal, "Address");
  setHeaderData(1, Qt::Horizontal, "+0");
  setHeaderData(2, Qt::Horizontal, "+1");
  setHeaderData(3, Qt::Horizontal, "+2");
  setHeaderData(4, Qt::Horizontal, "+3");
}
