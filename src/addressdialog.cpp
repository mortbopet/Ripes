#include "addressdialog.h"
#include "ui_addressdialog.h"

#include <QPushButton>
#include <QRegularExpressionValidator>

#include "processorhandler.h"
#include "radix.h"

namespace Ripes {

AddressDialog::AddressDialog(QWidget *parent)
    : QDialog(parent), m_ui(new Ui::AddressDialog) {
  m_ui->setupUi(this);

  QRegularExpressionValidator *validator =
      new QRegularExpressionValidator(this);
  setISADepRegex(validator);
  m_ui->address->setValidator(validator);
  m_ui->address->setText(
      "0x" +
      QString("0").repeated(ProcessorHandler::currentISA()->bytes() * 2));
  setWindowTitle("Ripes");

  connect(m_ui->address, &QLineEdit::textChanged, this,
          &AddressDialog::validateTargetAddress);
}

AddressDialog::~AddressDialog() { delete m_ui; }

void AddressDialog::validateTargetAddress(const QString &address) {
  bool ok;

  AInt value;
  const auto isaBytes = ProcessorHandler::currentISA()->bytes();
  if (isaBytes == 2) {
    value = address.toUShort(&ok, 16);
  } else if (isaBytes == 4) {
    value = address.toUInt(&ok, 16);
  } else if (isaBytes == 8) {
    value = address.toULongLong(&ok, 16);
  } else {
    Q_UNREACHABLE();
  }

  if (ok) {
    m_address = value;
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  } else {
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
}
} // namespace Ripes
