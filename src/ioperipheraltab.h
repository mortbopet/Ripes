#pragma once
#include <QWidget>

namespace Ripes {
class IOBase;

namespace Ui {
class IOPeripheralTab;
}

class IOPeripheralTab : public QWidget {
  Q_OBJECT

public:
  IOPeripheralTab(QWidget *parent, IOBase *peripheral);
  ~IOPeripheralTab();

private:
  void updateExportsInfo();

  Ui::IOPeripheralTab *m_ui = nullptr;
  IOBase *m_peripheral = nullptr;
};

} // namespace Ripes
