#include "addressdialog.h"
#include "ui_addressdialog.h"

#include <QPushButton>

AddressDialog::AddressDialog(QWidget *parent)
    : QDialog(parent), m_ui(new Ui::AddressDialog) {
  m_ui->setupUi(this);

  // set input range
  m_ui->address->setInputMask("hhhhhhhh");

  connect(m_ui->address, &QLineEdit::textChanged, this,
          &AddressDialog::validateTargetAddress);
}

AddressDialog::~AddressDialog() { delete m_ui; }

void AddressDialog::validateTargetAddress(const QString &address) {
  bool ok;
  uint32_t value = address.toUInt(&ok, 16);
  if (ok && value >= 0 && value <= (long long)4294967295) {
    m_address = value;
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  } else {
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
}
