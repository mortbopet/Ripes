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
    m_ui->binaryedit->setReadOnly(true);

    // enable breakpoint area for the translated code only
    m_ui->binaryedit->enableBreakpointArea();

    // Link scrollbars together for pleasant navigation
    connect(m_ui->assemblyedit->verticalScrollBar(), &QScrollBar::valueChanged, m_ui->binaryedit->verticalScrollBar(),
            &QScrollBar::setValue);
    connect(m_ui->binaryedit->verticalScrollBar(), &QScrollBar::valueChanged, m_ui->assemblyedit->verticalScrollBar(),
            &QScrollBar::setValue);

    m_assembler = new Assembler();

    connect(m_ui->assemblyedit, &CodeEditor::textChanged, this, &EditTab::assemble);
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
    setDisassemblerText();
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
    m_ui->binaryedit->clear();
    enableEditor();
    emit editorStateChanged(true);
}

void EditTab::setDisassemblerText() {
    const auto* textSection = m_activeProgram.getSection(TEXT_SECTION_NAME);
    if (!textSection)
        return;

    auto text = m_ui->disassembledViewButton->isChecked() ? Parser::getParser()->disassemble(textSection->data)
                                                          : Parser::getParser()->binarize(textSection->data);
    m_ui->binaryedit->setPlainText(text);
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
    emit editorStateChanged(false);
}

void EditTab::on_disassembledViewButton_toggled() {
    setDisassemblerText();
}

bool EditTab::loadFlatBinaryFile(Program& program, QFile& file, uint32_t entryPoint, uint32_t loadAt) {
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
        section.data = QByteArray(elfSection->get_data(), elfSection->get_size());
    }

    program.entryPoint = reader.get_entry();

    m_ui->curInputSrcLabel->setText("Executable (ELF)");
    m_ui->inputSrcPath->setText(file.fileName());

    disableEditor();
    return true;
}

}  // namespace Ripes
