#include "compilererrordialog.h"
#include "ui_compilererrordialog.h"

namespace Ripes {

CompilerErrorDialog::CompilerErrorDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::CompilerErrorDialog) {
    m_ui->setupUi(this);

    setWindowTitle("Compilation error");
}

void CompilerErrorDialog::setText(const QString& text) {
    m_ui->infoText->setText(text);
}
void CompilerErrorDialog::setErrorText(const QString& text) {
    m_ui->errorText->setPlainText(text);
}

CompilerErrorDialog::~CompilerErrorDialog() {
    delete m_ui;
}

}  // namespace Ripes
