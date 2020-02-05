#pragma once

#include <QWidget>

#include "processorhandler.h"
#include "registermodel.h"

namespace Ripes {

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget* parent = nullptr);
    ~RegisterWidget();

    void updateModel();

    RegisterModel* m_registerModel = nullptr;

public slots:
    void updateView();
    void setRegisterviewCenterIndex(int index);

private:
    Ui::RegisterWidget* m_ui = nullptr;
};
}  // namespace Ripes
