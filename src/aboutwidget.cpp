#include "aboutwidget.h"
#include "ui_aboutwidget.h"

namespace Ripes {

AboutWidget::AboutWidget(QWidget* parent) : QDialog(parent), ui(new Ui::AboutWidget) {
    ui->setupUi(this);
    ui->icon->setPixmap(QPixmap(":/icons/logo.svg"));
}

AboutWidget::~AboutWidget() {
    delete ui;
}
}  // namespace Ripes
