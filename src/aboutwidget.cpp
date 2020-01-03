#include "aboutwidget.h"
#include "ui_aboutwidget.h"

AboutWidget::AboutWidget(QWidget* parent) : QDialog(parent), ui(new Ui::AboutWidget) {
    ui->setupUi(this);
    ui->icon->setPixmap(QPixmap(":/icons/logo.svg"));
}

AboutWidget::~AboutWidget() {
    delete ui;
}
