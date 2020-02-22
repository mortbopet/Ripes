#include "edittab.h"
#include "ui_edittab.h"

#include "elfio/elfio.hpp"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "parser.h"
#include "processorhandler.h"
#include "program.h"

namespace Ripes {

EditTab::EditTab(QToolBar* toolbar, QWidget* parent) : RipesTab(toolbar, parent), m_ui(new Ui::EditTab) {
    m_ui->setupUi(this);

    connect(m_ui->enableEditor, &QPushButton::clicked, this, &EditTab::enableAssemblyInput);

    // Only add syntax highlighter for code edit view - not for translated code. This is assumed to be correct after a
    // translation is complete
    m_ui->assemblyedit->setupSyntaxHighlighter();
    m_ui->assemblyedit->setupChangedTimer();
    m_ui->programViewer->setReadOnly(true);

    m_assembler = std::make_unique<Assembler>();

    connect(m_ui->assemblyedit, &CodeEditor::textChanged, this, &EditTab::assemble);

    enableEditor();
}

void EditTab::loadFile(const LoadFileParams& fileParams) {
    QFile file(fileParams.filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Error: Could not open file " + fileParams.filepath);
        return;
    }

    bool success = true;
    Program loadedProgram;
    switch (fileParams.type) {
        case FileType::Assembly:
            success &= loadAssemblyFile(loadedProgram, file);
            break;
        case FileType::FlatBinary:
            success &= loadFlatBinaryFile(loadedProgram, file, fileParams.binaryEntryPoint, fileParams.binaryLoadAt);
            break;
        case FileType::Executable:
            success &= loadElfFile(loadedProgram, file);
            break;
    }

    if (success) {
        m_loadedFile = fileParams;
        m_activeProgram = loadedProgram;
        emitProgramChanged();
    } else {
        QMessageBox::warning(this, "Error", "Error: Could not load file " + fileParams.filepath);
    }
    file.close();
}

QString EditTab::getAssemblyText() {
    return m_ui->assemblyedit->toPlainText();
}

const QByteArray& EditTab::getBinaryData() {
    return m_assembler->getTextSegment();
}

void EditTab::clearAssemblyEditor() {
    m_ui->assemblyedit->reset();
    m_assembler->clear();
}

void EditTab::emitProgramChanged() {
    updateProgramViewer();
    emit programChanged(&m_activeProgram);
}

void EditTab::assemble() {
    if (m_ui->assemblyedit->syntaxAccepted()) {
        m_assembler->assemble(*m_ui->assemblyedit->document());
        if (!m_assembler->hasError()) {
            m_activeProgram = m_assembler->getProgram();
            emitProgramChanged();
        } else {
            QMessageBox err;
            err.setText("Error during assembling of program");
            err.exec();
        }
    }
}

EditTab::~EditTab() {
    delete m_ui;
}

void EditTab::newProgram() {
    m_ui->assemblyedit->reset();
    m_ui->assemblyedit->clear();
}

void EditTab::setAssemblyText(const QString& text) {
    m_ui->assemblyedit->reset();
    m_ui->assemblyedit->setPlainText(text);
}

void EditTab::enableAssemblyInput() {
    // Clear currently loaded binary/ELF program
    m_activeProgram = Program();
    m_ui->programViewer->clear();
    enableEditor();
    m_editorEnabled = true;
    emit editorStateChanged(m_editorEnabled);
}

void EditTab::updateProgramViewer() {
    m_ui->programViewer->updateProgram(m_activeProgram, !m_ui->disassembledViewButton->isChecked());
}

void EditTab::enableEditor() {
    connect(m_ui->assemblyedit, &CodeEditor::textChanged, this, &EditTab::assemble);
    m_ui->editorStackedWidget->setCurrentIndex(0);
    clearAssemblyEditor();
}

void EditTab::disableEditor() {
    disconnect(m_ui->assemblyedit, &CodeEditor::textChanged, this, &EditTab::assemble);
    m_ui->editorStackedWidget->setCurrentIndex(1);
    clearAssemblyEditor();
    m_editorEnabled = false;
    emit editorStateChanged(m_editorEnabled);
}

void EditTab::on_disassembledViewButton_toggled() {
    updateProgramViewer();
}

bool EditTab::loadFlatBinaryFile(Program& program, QFile& file, unsigned long entryPoint, unsigned long loadAt) {
    ProgramSection section;
    section.name = TEXT_SECTION_NAME;
    section.address = loadAt;
    section.data = file.readAll();

    program.sections.push_back(section);
    program.entryPoint = entryPoint;

    m_ui->curInputSrcLabel->setText("Flat binary");
    m_ui->inputSrcPath->setText(file.fileName());
    disableEditor();
    return true;
}

bool EditTab::loadAssemblyFile(Program&, QFile& file) {
    enableEditor();
    setAssemblyText(file.readAll());
    return true;
}

bool EditTab::loadElfFile(Program& program, QFile& file) {
    ELFIO::elfio reader;

    // No file validity checking is performed - it is expected that Loaddialog has done all validity
    // checking.
    if (!reader.load(file.fileName().toStdString())) {
        assert(false);
    }

    for (const auto& elfSection : reader.sections) {
        ProgramSection& section = program.sections.emplace_back();
        section.name = QString::fromStdString(elfSection->get_name());
        section.address = elfSection->get_address();
        // QByteArray performs a deep copy of the data when the data array is initialized at construction
        section.data = QByteArray(elfSection->get_data(), static_cast<int>(elfSection->get_size()));

        if (elfSection->get_type() == SHT_SYMTAB) {
            // Collect function symbols
            const ELFIO::symbol_section_accessor symbols(reader, elfSection);
            for (unsigned int j = 0; j < symbols.get_symbols_num(); ++j) {
                std::string name;
                ELFIO::Elf64_Addr value;
                ELFIO::Elf_Xword size;
                unsigned char bind;
                unsigned char type;
                ELFIO::Elf_Half section_index;
                unsigned char other;
                symbols.get_symbol(j, name, value, size, bind, type, section_index, other);

                if (type != STT_FUNC)
                    continue;
                program.symbols[value] = QString::fromStdString(name);
            }
        }
    }

    program.entryPoint = reader.get_entry();

    m_ui->curInputSrcLabel->setText("Executable (ELF)");
    m_ui->inputSrcPath->setText(file.fileName());

    disableEditor();
    return true;
}

}  // namespace Ripes
