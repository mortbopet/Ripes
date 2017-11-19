#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QDialog>

namespace Ui {
class AboutWidget;
}

class AboutWidget : public QDialog
{
    Q_OBJECT

public:
    explicit AboutWidget(QWidget *parent = 0);
    ~AboutWidget();

private:
    Ui::AboutWidget *ui;
};

#endif // ABOUTWIDGET_H
