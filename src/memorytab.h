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

private:
  Ui::MemoryTab *m_ui;
};

#endif // MEMORYTAB_H
