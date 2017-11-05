#ifndef MEMORYTAB_H
#define MEMORYTAB_H

#include <QWidget>

namespace Ui {
class MemoryTab;
}

class RegisterWidget;

class MemoryTab : public QWidget {
  Q_OBJECT

public:
  explicit MemoryTab(QWidget *parent = 0);
  ~MemoryTab();

  void setMemoryPtr(std::vector<uint8_t> *ptr) { m_memoryPtr = ptr; }
  void setRegPtr(std::vector<uint32_t> *ptr) { m_regPtr = ptr; }
  void init();

public slots:
  void updateRegisterWidget(int n);

private:
  Ui::MemoryTab *m_ui;

  std::vector<uint8_t> *m_memoryPtr;
  std::vector<uint32_t> *m_regPtr;

  std::vector<RegisterWidget *> m_regWidgetPtrs;
};

#endif // MEMORYTAB_H
