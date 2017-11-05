#include "registerwidget.h"
#include "ui_registerwidget.h"

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent), m_ui(new Ui::RegisterWidget) {
  m_ui->setupUi(this);
  // m_ui->value->setValidator(&m_validator);

  connect(m_ui->value, &QLineEdit::editingFinished, this,
          &RegisterWidget::validateInput);
}

RegisterWidget::~RegisterWidget() { delete m_ui; }

void RegisterWidget::setAlias(QString text) { m_ui->alias->setText(text); }

void RegisterWidget::validateInput() {
  // Instead of subclassing QValidator for ie. hex, we do some simple input
  // validation here
  const QString input = m_ui->value->text();

  bool ok;
  auto value = input.toLongLong(&ok, m_displayBase);
  if (ok && value >= m_range.first &&
      value <= m_range.second) { // verify "ok" and that value is within current
                                 // accepted range of the display type
    *m_regPtr = value;
  } else {
    // revert lineedit to the current register value
    setText();
  }
}

void RegisterWidget::setText() {
  // Sets line edit text based on current display type and register value
  if (m_displayType == displayTypes[0]) {
    // hex
    m_ui->value->setText(
        QString()
            .setNum(*m_regPtr, 16)
            .rightJustified(8, '0')); // zero padding on hex numbers
  } else if (m_displayType == displayTypes[1]) {
    // binary
    m_ui->value->setText(QString().setNum(*m_regPtr, 2));
  } else if (m_displayType == displayTypes[2]) {
    // Decimal
    m_ui->value->setText(QString().setNum(*(int32_t *)m_regPtr, 10));
  } else if (m_displayType == displayTypes[3]) {
    // Unsigned
    m_ui->value->setText(QString().setNum(*m_regPtr, 10));
  } else if (m_displayType == displayTypes[4]) {
    // ASCII - valid ascii characters will not be able to fill the 32-bit range
    m_ui->value->setInputMask("nnnn");
  } else if (m_displayType == displayTypes[5]) {
    // float
    m_ui->value->setInputMask("B");
  }
}

void RegisterWidget::setNumber(int number) {
  m_ui->number->setText(QString("x(%1)").arg(number));
}

void RegisterWidget::setDisplayType(QString type) {
  // Given a display type "type", sets validators for the input.

  m_displayType = type;
  if (type == displayTypes[0]) {
    // hex
    m_displayBase = 16;
    m_ui->value->setInputMask("hhhhhhhh");
    m_range = rangePair(0, (long long)4294967295);
    setText();
  } else if (type == displayTypes[1]) {
    // binary
    m_displayBase = 2;
    m_ui->value->setInputMask("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
    setText();
    m_range = rangePair(0, (long long)4294967295);
  } else if (type == displayTypes[2]) {
    // Decimal
    m_displayBase = 10;
    m_ui->value->setInputMask("#0000000000");
    setText();
    m_range = rangePair(-(long long)2147483648, (long long)2147483647);
  } else if (type == displayTypes[3]) {
    // Unsigned
    m_displayBase = 10;
    m_ui->value->setInputMask("0000000000");
    setText();
    m_range = rangePair(0, (long long)4294967295);
  } else if (type == displayTypes[4]) {
    // ASCII - valid ascii characters will not be able to fill the 32-bit range
    m_ui->value->setInputMask("nnnn");
    m_range = rangePair(0, (long long)4294967295);
  } else if (type == displayTypes[5]) {
    // float
    m_ui->value->setInputMask("B");
    m_range = {0, (long)4294967295};
  }
}

void RegisterWidget::enableInput(bool state) {
  // permanently called on reg[0], and on all registers when running simulation,
  // to disable memory editing while running
  m_ui->value->setEnabled(state);
}
