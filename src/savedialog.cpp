#include "savedialog.h"
#include "ui_savedialog.h"

#include <QCheckBox>
#include <QFileDialog>

namespace Ripes {

SaveDialog::SaveDialog(SourceType sourceType, QWidget* parent)
    : QDialog(parent), m_ui(new Ui::SaveDialog), sourceType(sourceType) {
    m_ui->setupUi(this);

    setWindowTitle("Save files...");

    connect(m_ui->openFile, &QPushButton::clicked, this, &SaveDialog::openFileButtonTriggered);
    connect(m_ui->filePath, &QLineEdit::textChanged, this, &SaveDialog::pathChanged);
    connect(m_ui->saveBinary, &QCheckBox::toggled, this, &SaveDialog::pathChanged);
    connect(m_ui->saveSource, &QCheckBox::toggled, this, &SaveDialog::pathChanged);

    m_ui->saveSource->setChecked(RipesSettings::value(RIPES_SETTING_SAVE_SOURCE).toBool());
    m_ui->saveBinary->setChecked(RipesSettings::value(RIPES_SETTING_SAVE_BINARY).toBool());

    QString path = RipesSettings::value(RIPES_SETTING_SAVEPATH).toString();
    if (!QFileInfo(path).dir().exists()) {
        path = QDir::currentPath();
    }
    if (QFileInfo(path).isDir()) {
        path += QDir::separator();
    }
    m_ui->filePath->setText(path);

    pathChanged();
}

void SaveDialog::accept() {
    RipesSettings::setValue(RIPES_SETTING_SAVE_SOURCE, m_ui->saveSource->isChecked());
    RipesSettings::setValue(RIPES_SETTING_SAVE_BINARY, m_ui->saveBinary->isChecked());
    RipesSettings::setValue(RIPES_SETTING_SAVEPATH, m_ui->filePath->text());

    QDialog::accept();
}

void SaveDialog::pathChanged() {
    bool okEnabled = !QFileInfo(m_ui->filePath->text()).fileName().isEmpty();
    okEnabled &= m_ui->saveSource->isChecked() | m_ui->saveBinary->isChecked();

    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(okEnabled);

    QStringList filesToSave;
    const QString& path = m_ui->filePath->text();
    if (m_ui->saveSource->isChecked() && !path.isEmpty() &&
        (sourceType == SourceType::Assembly || sourceType == SourceType::C)) {
        filesToSave << m_ui->filePath->text() + (sourceType == SourceType::Assembly ? ".s" : ".c");
    }
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
