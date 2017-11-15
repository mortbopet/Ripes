#ifndef PROCESSORTAB_H
#define PROCESSORTAB_H

#include <QWidget>

namespace Ui {
class ProcessorTab;
}

class ProcessorTab : public QWidget {
    Q_OBJECT

  public:
    explicit ProcessorTab(QWidget* parent = 0);
    ~ProcessorTab();

  private:
    Ui::ProcessorTab* m_ui;
};

#endif // PROCESSORTAB_H
