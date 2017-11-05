#ifndef MEMORYTAB_H
#define MEMORYTAB_H

#include <QWidget>

namespace Ui {
class MemoryTab;
}

class MemoryTab : public QWidget {
  Q_OBJECT

public:
  explicit MemoryTab(QWidget *parent = 0);
  ~MemoryTab();

  void setMemoryPtr(uint32_t *ptr) { m_memoryPtr = ptr; }
  void setRegPtr(std::vector<uint32_t> *ptr) { m_regPtr = ptr; }

private:
  Ui::MemoryTab *m_ui;

  uint32_t *m_memoryPtr;
  std::vector<uint32_t> *m_regPtr;
};

#endif // MEMORYTAB_H
