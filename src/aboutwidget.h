#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QDialog>

namespace Ui {
class AboutWidget;
}

class AboutWidget : public QDialog {
    Q_OBJECT

public:
    explicit AboutWidget(QWidget* parent = nullptr);
    ~AboutWidget() override;

private:
    Ui::AboutWidget* ui;
};

#endif  // ABOUTWIDGET_H
