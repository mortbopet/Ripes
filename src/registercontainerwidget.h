#ifndef REGISTERCONTAINERWIDGET_H
#define REGISTERCONTAINERWIDGET_H

#include <QWidget>

namespace Ui {
class RegisterContainerWidget;
}

class RegisterWidget;
class RegisterContainerWidget : public QWidget {
    Q_OBJECT
public:
    RegisterContainerWidget(QWidget* parent = nullptr);

    void setRegPtr(std::vector<uint32_t>* regPtr) { m_regPtr = regPtr; }
    void init();

public slots:
    void update();
    void registerHasChanged();

private:
    Ui::RegisterContainerWidget* m_ui;

    std::vector<uint32_t>* m_regPtr;
    QList<RegisterWidget*> m_registers;
    RegisterWidget* m_currentHighlightedReg = nullptr;
};

#endif  // REGISTERCONTAINERWIDGET_H
