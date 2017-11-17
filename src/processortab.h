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

    void initRegWidget(std::vector<uint32_t>* regPtr);

   private:
    Ui::ProcessorTab* m_ui;
};

#endif  // PROCESSORTAB_H
