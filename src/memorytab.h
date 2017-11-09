#ifndef MEMORYTAB_H
#define MEMORYTAB_H

#include "defines.h"

#include <QWidget>

#include <unordered_map>

#include "memorymodel.h"

namespace Ui {
class MemoryTab;
}

class RegisterWidget;

class MemoryTab : public QWidget {
  Q_OBJECT

public:
  explicit MemoryTab(QWidget *parent = 0);
  ~MemoryTab();

  void setMemoryPtr(memory *ptr) { m_memoryPtr = ptr; }
  void setRegPtr(std::vector<uint32_t> *ptr) { m_regPtr = ptr; }
  void init();

public slots:
  void updateRegisterWidget(int n);

private:
  void initializeMemoryView();
  void initializeRegisterView();

  Ui::MemoryTab *m_ui;
  MemoryModel *m_model;

  memory *m_memoryPtr;
  std::vector<uint32_t> *m_regPtr;

  std::vector<RegisterWidget *> m_regWidgetPtrs;
};

#endif // MEMORYTAB_H
