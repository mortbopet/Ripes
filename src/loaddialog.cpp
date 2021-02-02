#include "loaddialog.h"
#include "ui_loaddialog.h"

#include "elfinfostrings.h"
#include "elfio/elfio.hpp"

#include "processorhandler.h"
#include "assembler/program.h"
#include "radix.h"

#include <QButtonGroup>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QRegExpValidator>

namespace Ripes {

LoadDialog::TypeButtonID LoadDialog::s_typeIndex = TypeButtonID::ELF;
QString LoadDialog::s_filePath = QString();

LoadDialog::LoadDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::LoadDialog) {
    m_ui->setupUi(this);

    setWindowTitle("Load program...");

    m_fileTypeButtons = new QButtonGroup(this);
    m_fileTypeButtons->addButton(m_ui->sourceRadioButton, TypeButtonID::Source);
    m_fileTypeButtons->addButton(m_ui->binaryRadioButton, TypeButtonID::FlatBinary);
    m_fileTypeButtons->addButton(m_ui->elfRadioButton, TypeButtonID::ELF);

    connect(m_fileTypeButtons, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), this,
            &LoadDialog::inputTypeChanged);
    connect(m_ui->openFile, &QPushButton::clicked, this, &LoadDialog::openFileButtonTriggered);
    connect(m_ui->filePath, &QLineEdit::textChanged, this, &LoadDialog::validateCurrentFile);

    // ===================== Page setups =====================

    // Source page
    // -- nothing to do/verify

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
    m_fileTypeButtons->button(s_typeIndex)->toggle();
    m_ui->filePath->setText(s_filePath);
}

void LoadDialog::inputTypeChanged() {
    s_typeIndex = static_cast<LoadDialog::TypeButtonID>(m_fileTypeButtons->checkedId());
    switch (m_fileTypeButtons->checkedId()) {
        case TypeButtonID::Source: {
            m_currentType = TypeButtonID::Source;
            updateSourcePageState();
            break;
        }
        case TypeButtonID::FlatBinary: {
            m_currentType = TypeButtonID::FlatBinary;
            updateBinaryPageState();
            break;
        }
        case TypeButtonID::ELF: {
            m_currentType = TypeButtonID::ELF;
            updateELFPageState();
            break;
        }
    }
    validateCurrentFile();
}

void LoadDialog::openFileButtonTriggered() {
    QString title;
    QString filter;
    switch (m_currentType) {
        case TypeButtonID::Source: {
            title = "Open source file";
            filter = "Source files [*.s, *.as, *.asm, *.c] (*.s *.as *.asm *.c);; All files (*.*)";
            break;
        }
        case TypeButtonID::FlatBinary: {
            title = "Open binary file";
            filter = "All files (*)";
            break;
        }
        case TypeButtonID::ELF: {
            title = "Open executable (ELF) file";
            filter = "All files (*)";
            break;
        }
    }

    const auto filename = QFileDialog::getOpenFileName(this, title, "", filter);
    if (!filename.isEmpty()) {
        m_ui->filePath->setText(filename);
    }
}

void LoadDialog::paletteValidate(QWidget* w, bool valid) {
    QPalette palette = this->palette();
    if (!valid) {
        palette.setColor(QPalette::Base, QColor("#eb8383"));
    }
    w->setPalette(palette);
}

bool LoadDialog::validateSourceFile(const QFile&) {
    return true;
}

bool LoadDialog::validateBinaryFile(const QFile&) {
    bool loadAtValid, entryPointValid;
    m_ui->binaryLoadAt->text().toUInt(&loadAtValid, 16);
    paletteValidate(m_ui->binaryLoadAt, loadAtValid);

    m_ui->binaryEntryPoint->text().toUInt(&entryPointValid, 16);
    paletteValidate(m_ui->binaryEntryPoint, entryPointValid);

    return loadAtValid && entryPointValid;
}

void LoadDialog::setElfInfo(const ELFInfo& info) {
    if (info.valid) {
        m_ui->elfInfo->clear();
    } else {
        m_ui->elfInfo->setText("<b>Error:</b> " + info.errorMessage);
    }
}

ELFInfo LoadDialog::validateELFFile(const QFile& file) {
    ELFIO::elfio reader;
    ELFInfo info;
    QString flagErr;
    unsigned elfbits;
    info.valid = true;

    // Is it an ELF file?
    if (!reader.load(file.fileName().toStdString())) {
        info.errorMessage = "Not an ELF file";
        info.valid = false;
        goto finish;
    }

    // Is it a compatible machine format?
    if (reader.get_machine() != ProcessorHandler::get()->currentISA()->elfMachineId()) {
        info.errorMessage = "Incompatible ELF machine type (ISA).<br/><br/>Expected machine type:<br/>'" +
                            QString::number(ProcessorHandler::get()->currentISA()->elfMachineId()) + "' (" +
                            getNameForElfMachine(ProcessorHandler::get()->currentISA()->elfMachineId()) +
                            ")<br/>but file has machine type:<br/>    '" + QString::number(reader.get_machine()) +
                            "' (" + getNameForElfMachine(reader.get_machine()) + ")";
        info.valid = false;
        goto finish;
    }

    // Is it a compatible file class?
    elfbits = reader.get_class() == ELFCLASS32 ? 32 : 64;
    if (elfbits != ProcessorHandler::get()->currentISA()->bits()) {
        const QString bitSize = elfbits == 32 ? "32" : "64";
        info.errorMessage = "Expected " + QString::number(ProcessorHandler::get()->currentISA()->bits()) +
                            " bit executable, but input file is a " + bitSize + " bit executable.";
        info.valid = false;
        goto finish;
    }

    // executable? (Not dynamically linked nor relocateable)
    if (!(reader.get_type() == ET_EXEC)) {
        info.errorMessage = "Only executable ELF files are supported.<br/><br/>File type is<br/>" +
                            QString::number(reader.get_type()) + " (" + getNameForElfType(reader.get_type()) +
                            ")<br/>Expected<br/>" + QString::number(ET_EXEC) + " (" + getNameForElfType(ET_EXEC) + ")";
        info.valid = false;
        goto finish;
    }

    // Supported flags?
    flagErr = ProcessorHandler::get()->currentISA()->elfSupportsFlags(reader.get_flags());
    if (!flagErr.isEmpty()) {
        info.errorMessage = flagErr;
        info.valid = false;
        goto finish;
    }

    // All checks successfull - ELF file is valid.

finish:

    return info;
}

bool LoadDialog::fileTypeValidate(const QFile& file) {
    switch (m_currentType) {
        case TypeButtonID::Source:
            return validateSourceFile(file);
        case TypeButtonID::FlatBinary:
            return validateBinaryFile(file);
        case TypeButtonID::ELF:
            auto info = validateELFFile(file);
            setElfInfo(info);
            return info.valid;
    }
    Q_UNREACHABLE();
}

void LoadDialog::validateCurrentFile() {
    const QString& filename = m_ui->filePath->text();
    s_filePath = filename;
    QFile file(filename);
    const bool filePathValid = file.exists();
    bool fileTypeValid = false;
    if (filePathValid) {
        fileTypeValid = fileTypeValidate(file);
    }

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
    switch (m_currentType) {
        case TypeButtonID::Source:
            // Set source type based on file extension
            m_params.type = m_params.filepath.endsWith(".c") ? SourceType::C : SourceType::Assembly;
            break;
        case TypeButtonID::FlatBinary:
            m_params.type = SourceType::FlatBinary;
            break;
        case TypeButtonID::ELF:
            m_params.type = SourceType::ExternalELF;
            break;
    }

    m_params.binaryLoadAt = m_ui->binaryLoadAt->text().toUInt(nullptr, 16);
    m_params.binaryEntryPoint = m_ui->binaryEntryPoint->text().toUInt(nullptr, 16);

    QDialog::accept();
}

void LoadDialog::updateSourcePageState() {
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
