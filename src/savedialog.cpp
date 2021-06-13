#include "savedialog.h"
#include "ui_savedialog.h"

#include <QCheckBox>
#include <QFileDialog>

namespace Ripes {

SaveDialog::SaveDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::SaveDialog) {
    m_ui->setupUi(this);

    setWindowTitle("Save files...");

    connect(m_ui->openFile, &QPushButton::clicked, this, &SaveDialog::openFileButtonTriggered);
    connect(m_ui->filePath, &QLineEdit::textChanged, this, &SaveDialog::pathChanged);
    connect(m_ui->saveBinary, &QCheckBox::toggled, this, &SaveDialog::pathChanged);
    connect(m_ui->saveAssembly, &QCheckBox::toggled, this, &SaveDialog::pathChanged);

    m_ui->saveAssembly->setChecked(RipesSettings::value(RIPES_SETTING_SAVE_ASSEMBLY).toBool());
    m_ui->saveBinary->setChecked(RipesSettings::value(RIPES_SETTING_SAVE_BINARY).toBool());
    m_ui->filePath->setText(RipesSettings::value(RIPES_SETTING_SAVEPATH).toString());
    pathChanged();
}

void SaveDialog::accept() {
    RipesSettings::setValue(RIPES_SETTING_SAVE_ASSEMBLY, m_ui->saveAssembly->isChecked());
    RipesSettings::setValue(RIPES_SETTING_SAVE_BINARY, m_ui->saveBinary->isChecked());
    RipesSettings::setValue(RIPES_SETTING_SAVEPATH, m_ui->filePath->text());

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
        m_ui->filePath->setText(dialog.selectedFiles().at(0));
    }
}

SaveDialog::~SaveDialog() {
    delete m_ui;
}

}  // namespace Ripes
