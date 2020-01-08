#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>

#include "processorhandler.h"
#include "registermodel.h"

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget {
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget* parent = nullptr);
    ~RegisterWidget();

    void updateModel();
    void setHandler(ProcessorHandler* handler);

    RegisterModel* m_registerModel = nullptr;

public slots:
    void updateView();

private:
    void setupRadixComboBox();
    void updateRadixComboBoxIndex();

    Ui::RegisterWidget* m_ui;
    ProcessorHandler* m_handler = nullptr;
};

#endif  // REGISTERWIDGET_H
