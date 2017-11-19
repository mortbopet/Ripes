#include "aboutwidget.h"
#include "ui_aboutwidget.h"

AboutWidget::AboutWidget(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutWidget) {
    ui->setupUi(this);

    ui->description->setText(

      "A 5-stage RISC-V pipeline simulator with added assembly development "
      "environment. The simulator includes features such as forwarding, "
      "branching strategy selection, assembly development environment, "
      "breakpoint setting and more."
      "\n\nDeveloped as part of a 3-week project course at DTU - The Technical "
      "University of Denmark.");
}

AboutWidget::~AboutWidget() { delete ui; }
