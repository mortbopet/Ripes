#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = 0);
    ~RegisterWidget();

private:
    Ui::RegisterWidget *ui;
};

#endif // REGISTERWIDGET_H
