#ifndef MEMORYMODEL_H
#define MEMORYMODEL_H

#include "defines.h"

#include <QStandardItemModel>

#include <unordered_map>

class MemoryModel : public QStandardItemModel {
  Q_OBJECT

public:
  explicit MemoryModel(memory *memoryPtr, QObject *parent = nullptr);

  // Custom functionality
  void setCentralAddress(uint32_t address);
  void jumpToAddress(uint32_t address);
  void moveSelection(int dir);

  void setAddressCount(int count);

private:
  // instead of direct access to the memory, we should have a pointer
  // to the runner, which can return requested values, or null, if memory
  // doesnt exist
  memory *m_memoryPtr;

  uint32_t m_centralAddress = 0; // Address at the center of the model
  int m_addressRadius = 5;       // amount of addresses in each direction, from
                                 // centralAddress
};

#endif // MEMORYMODEL_H
