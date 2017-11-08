#ifndef MEMORYTAB_H
#define MEMORYTAB_H

#include <QWidget>
#include <unordered_map>

namespace Ui {
class MemoryTab;
}

class RegisterWidget;

class MemoryTab : public QWidget {
  Q_OBJECT

public:
  explicit MemoryTab(QWidget *parent = 0);
  ~MemoryTab();

  void setMemoryPtr(std::unordered_map<uint32_t, uint8_t> *ptr) {
    m_memoryPtr = ptr;
  }
  void setRegPtr(std::vector<uint32_t> *ptr) { m_regPtr = ptr; }
  void init();

public slots:
  void updateRegisterWidget(int n);

private:
  void initializeMemoryView();

  Ui::MemoryTab *m_ui;

  std::unordered_map<uint32_t, uint8_t> *m_memoryPtr;
  std::vector<uint32_t> *m_regPtr;

  std::vector<RegisterWidget *> m_regWidgetPtrs;
};

#endif // MEMORYTAB_H
