#include "memorymodel.h"

MemoryModel::MemoryModel(memory *memoryPtr, QObject *parent)
    : QStandardItemModel(parent) {
  m_memoryPtr = memoryPtr;

  // Create initial view
  // i is a byte index
  for (int i = 0; i < m_addressRadius * 2 + 1; i++) {
    auto lineAddress = m_centralAddress + m_addressRadius - i * 4;
    auto iter = m_memoryPtr->find(lineAddress);
    if (iter != m_memoryPtr->end()) {
      setItem(i, 0, new QStandardItem(iter->first));
      setItem(i, 1, new QStandardItem(iter->second));
    }
    for (int j = 1; j < 4; j++) {
      // Locate the remaining 3 bytes in the word
      if ((iter = m_memoryPtr->find(lineAddress + j)) != m_memoryPtr->end()) {
        setItem(i, j + 1, new QStandardItem(iter->second));
      } else {
        // Memory is not initialized, set model data value to 0
        setItem(i, j + 1, 0);
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
