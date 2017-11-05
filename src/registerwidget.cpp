#include "registerwidget.h"
#include "ui_registerwidget.h"

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::RegisterWidget) {
  m_ui->setupUi(this);
  m_ui->value->setValidator(&m_validator);
}

RegisterWidget::~RegisterWidget() { delete m_ui; }

void RegisterWidget::setAlias(QString text) { m_ui->alias->setText(text); }

void RegisterWidget::setNumber(int number) {
  m_ui->number->setText(QString("x(%1)").arg(number));
}

void RegisterWidget::setDisplayType(QString type) {
  if (type == displayTypes[0]) {
    // hex
    m_ui->value->setInputMask("HHHHHHHH");
    m_ui->value->setText(QString().setNum(*m_regPtr, 16));
    // m_validator.setRange(0, 4294967295);
  } else if (type == displayTypes[1]) {
    // binary
    m_ui->value->setInputMask("BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
    m_ui->value->setText(QString().setNum(*m_regPtr, 2));
    // m_validator.setRange(0, 4294967295);
  } else if (type == displayTypes[2]) {
    // Decimal
    m_ui->value->setInputMask("#9999999999");
    m_ui->value->setText(QString().setNum((int32_t)*m_regPtr, 10));
    // m_validator.setRange(-2147483648, 2147483647);
  } else if (type == displayTypes[3]) {
    // Unsigned
    m_ui->value->setInputMask("9999999999");
    m_ui->value->setText(QString().setNum(*m_regPtr, 10));
    // m_validator.setRange(0, 4294967295);
  } else if (type == displayTypes[4]) {
    // ASCII - valid ascii characters will not be able to fill the 32-bit range
    m_ui->value->setInputMask("NNNN");
    // m_validator.setRange(0, 4294967295);
  } else if (type == displayTypes[5]) {
    // float
    m_ui->value->setInputMask("B");
    // m_validator.setRange(0, 4294967295);
  }
}
