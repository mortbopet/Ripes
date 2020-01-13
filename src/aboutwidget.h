#pragma once

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
