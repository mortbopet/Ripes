#include "registerwidget.h"
#include "ui_registerwidget.h"

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::RegisterWidget) {
  m_ui->setupUi(this);
}

RegisterWidget::~RegisterWidget() { delete m_ui; }

void RegisterWidget::setAlias(QString text) { m_ui->alias->setText(text); }

void RegisterWidget::setNumber(int number) {
  m_ui->number->setText(QString("x(%1)").arg(number));
}
