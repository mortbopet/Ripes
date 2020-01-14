#include "loaddialog.h"
#include "ui_loaddialog.h"

#include "processorhandler.h"
#include "program.h"
#include "radix.h"

#include <QButtonGroup>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QRegExpValidator>

namespace Ripes {

LoadDialog::LoadDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::LoadDialog) {
    m_ui->setupUi(this);

    setWindowTitle("Load program...");

    m_fileTypeButtons = new QButtonGroup(this);
    m_fileTypeButtons->addButton(m_ui->assemblyRadioButton);
    m_fileTypeButtons->addButton(m_ui->binaryRadioButton);
    m_fileTypeButtons->addButton(m_ui->elfRadioButton);

    connect(m_fileTypeButtons, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), this,
            &LoadDialog::inputTypeChanged);
    connect(m_ui->openFile, &QPushButton::clicked, this, &LoadDialog::openFileButtonTriggered);
    connect(m_ui->filePath, &QLineEdit::textChanged, this, &LoadDialog::validateCurrentFile);

    // ===================== Page setups =====================

    // Assembly page

    // Binary page
    QRegExpValidator* validator = new QRegExpValidator(this);
    validator->setRegExp(hexRegex32);
    m_ui->binaryLoadAt->setValidator(validator);
    m_ui->binaryLoadAt->setText("0x00000000");
    m_ui->binaryEntryPoint->setValidator(validator);
    m_ui->binaryEntryPoint->setText("0x00000000");
    connect(m_ui->binaryLoadAt, &QLineEdit::textChanged, [=] { this->validateCurrentFile(); });
    connect(m_ui->binaryEntryPoint, &QLineEdit::textChanged, [=] { this->validateCurrentFile(); });

    // ELF page
    m_ui->currentISA->setText(ProcessorHandler::get()->currentISA()->name());

    // default selection
    m_ui->assemblyRadioButton->toggle();
    validateCurrentFile();
}

void LoadDialog::inputTypeChanged() {
    auto* button = m_fileTypeButtons->checkedButton();
    if (button == m_ui->assemblyRadioButton) {
        m_fileType = FileType::Assembly;
        updateAssemblyPageState();
    } else if (button == m_ui->binaryRadioButton) {
        m_fileType = FileType::FlatBinary;
        updateBinaryPageState();
    } else if (button == m_ui->elfRadioButton) {
        m_fileType = FileType::Executable;
        updateELFPageState();
    }
    validateCurrentFile();
}

void LoadDialog::openFileButtonTriggered() {
    QString title;
    QString filter;
    switch (m_fileType) {
        case FileType::Assembly: {
            title = "Open assembly file";
            filter = "Assembly files [*.s, *.as, *.asm] (*.s *.as *.asm);; All files (*.*)";
            break;
        }
        case FileType::FlatBinary: {
            title = "Open binary file";
            filter = "All files (*)";
            break;
        }
        case FileType::Executable: {
            title = "Open executable (ELF) file";
            filter = "All files (*)";
            break;
        }
    }

    const auto filename = QFileDialog::getOpenFileName(this, title, "", filter);
    m_ui->filePath->setText(filename);
}

void LoadDialog::paletteValidate(QWidget* w, bool valid) {
    QPalette palette = this->palette();
    if (!valid) {
        palette.setColor(QPalette::Base, QColor("#eb8383"));
    }
    w->setPalette(palette);
}

bool LoadDialog::validateAssemblyFile(const QFile& file) {
    return true;
}

bool LoadDialog::validateBinaryFile(const QFile& file) {
    bool loadAtValid, entryPointValid;
    m_ui->binaryLoadAt->text().toUInt(&loadAtValid, 16);
    paletteValidate(m_ui->binaryLoadAt, loadAtValid);

    m_ui->binaryEntryPoint->text().toUInt(&entryPointValid, 16);
    paletteValidate(m_ui->binaryEntryPoint, entryPointValid);

    return loadAtValid && entryPointValid;
}

bool LoadDialog::validateELFFile(const QFile& file) {
    return true;
}

bool LoadDialog::fileTypeValidate(const QFile& file) {
    switch (m_fileType) {
        case FileType::Assembly:
            return validateAssemblyFile(file);
        case FileType::FlatBinary:
            return validateBinaryFile(file);
        case FileType::Executable:
            return validateELFFile(file);
    }
}

void LoadDialog::validateCurrentFile() {
    const QString& filename = m_ui->filePath->text();
    QFile file(filename);
    const bool filePathValid = file.exists();
    const bool fileTypeValid = fileTypeValidate(file);

    paletteValidate(m_ui->filePath, filePathValid);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(filePathValid & fileTypeValid);
}

void LoadDialog::loadFileError(const QString& filename) {
    QMessageBox::warning(this, "Load file error", "Error: Could not load file \"" + filename + "\"");
    QDialog::reject();
}

void LoadDialog::accept() {
    // It is assumed that the currently selected file will always be valid for the currently selected type, if the
    // accept button is enabled. No further validation is performed.
    m_params.filepath = m_ui->filePath->text();
    m_params.type = m_fileType;
    m_params.binaryLoadAt = m_ui->binaryLoadAt->text().toUInt(nullptr, 16);
    m_params.binaryEntryPoint = m_ui->binaryEntryPoint->text().toUInt(nullptr, 16);

    QDialog::accept();
}

void LoadDialog::updateAssemblyPageState() {
    m_ui->fileTypePages->setCurrentIndex(0);
}

void LoadDialog::updateBinaryPageState() {
    m_ui->fileTypePages->setCurrentIndex(1);
}
void LoadDialog::updateELFPageState() {
    m_ui->fileTypePages->setCurrentIndex(2);
}

LoadDialog::~LoadDialog() {
    delete m_ui;
}

}  // namespace Ripes
