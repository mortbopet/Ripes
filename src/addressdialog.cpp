#include "addressdialog.h"
#include "ui_addressdialog.h"

#include <QPushButton>
#include <QRegExpValidator>

#include "radix.h"

AddressDialog::AddressDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::AddressDialog) {
    m_ui->setupUi(this);

    QRegExpValidator* validator = new QRegExpValidator(this);
    validator->setRegExp(hexRegex32);
    m_ui->address->setValidator(validator);
    m_ui->address->setText("0x00000000");
    setWindowTitle("Ripes");

    connect(m_ui->address, &QLineEdit::textChanged, this, &AddressDialog::validateTargetAddress);
}

AddressDialog::~AddressDialog() {
    delete m_ui;
}

void AddressDialog::validateTargetAddress(const QString& address) {
    bool ok;
    uint32_t value = address.toUInt(&ok, 16);
    if (ok) {
        m_address = value;
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    } else {
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}
