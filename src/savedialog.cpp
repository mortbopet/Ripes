#include "savedialog.h"
#include "ui_savedialog.h"

#include <QCheckBox>
#include <QFileDialog>

namespace Ripes {

QString SaveDialog::m_path = QString();
bool SaveDialog::m_saveAssembly = true;
bool SaveDialog::m_saveBinary = false;

SaveDialog::SaveDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::SaveDialog) {
    m_ui->setupUi(this);

    setWindowTitle("Save files...");

    connect(m_ui->openFile, &QPushButton::clicked, this, &SaveDialog::openFileButtonTriggered);
    connect(m_ui->filePath, &QLineEdit::textChanged, this, &SaveDialog::pathChanged);
    connect(m_ui->saveBinary, &QCheckBox::toggled, this, &SaveDialog::pathChanged);
    connect(m_ui->saveAssembly, &QCheckBox::toggled, this, &SaveDialog::pathChanged);

    m_ui->saveAssembly->setChecked(m_saveAssembly);
    m_ui->saveBinary->setChecked(m_saveBinary);
    m_ui->filePath->setText(m_path);
    pathChanged();
}

void SaveDialog::accept() {
    m_saveAssembly = m_ui->saveAssembly->isChecked();
    m_saveBinary = m_ui->saveBinary->isChecked();
    m_path = m_ui->filePath->text();

    QDialog::accept();
}

void SaveDialog::pathChanged() {
    bool okEnabled = !m_ui->filePath->text().isEmpty();
    okEnabled &= m_ui->saveAssembly->isChecked() | m_ui->saveBinary->isChecked();

    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(okEnabled);

    QStringList filesToSave;
    const QString& path = m_ui->filePath->text();
    if (m_ui->saveAssembly->isChecked() && !path.isEmpty())
        filesToSave << m_ui->filePath->text() + ".s";
    if (m_ui->saveBinary->isChecked() && !path.isEmpty())
        filesToSave << m_ui->filePath->text() + ".bin";

    m_ui->filesToSave->setText(filesToSave.join("<br/>"));
}

void SaveDialog::openFileButtonTriggered() {
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec()) {
        m_ui->filePath->setText(dialog.selectedFiles()[0]);
    }
}

SaveDialog::~SaveDialog() {
    delete m_ui;
}

}  // namespace Ripes
