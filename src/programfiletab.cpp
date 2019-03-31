#include "programfiletab.h"
#include "ui_programfiletab.h"

#include "parser.h"
#include "pipeline.h"

ProgramfileTab::ProgramfileTab(QWidget* parent) : QWidget(parent), m_ui(new Ui::ProgramfileTab) {
    m_ui->setupUi(this);

    // Only add syntax highlighter for code edit view - not for translated code. This is assumed to be correct after a
    // translation is complete
    m_ui->assemblyedit->setupSyntaxHighlighter();
    m_ui->assemblyedit->setupAssembler();
    m_ui->binaryedit->setReadOnly(true);
    // enable breakpoint area for the translated code only
    m_ui->binaryedit->enableBreakpointArea();

    // Link scrollbars together for pleasant navigation
    connect(m_ui->assemblyedit->verticalScrollBar(), &QScrollBar::valueChanged, m_ui->binaryedit->verticalScrollBar(),
            &QScrollBar::setValue);
    connect(m_ui->binaryedit->verticalScrollBar(), &QScrollBar::valueChanged, m_ui->assemblyedit->verticalScrollBar(),
            &QScrollBar::setValue);

    // Connect data parsing signals from the assembler to this
    connect(m_ui->assemblyedit, &CodeEditor::assembledSuccessfully, this, &ProgramfileTab::assemblingComplete);
}

QString ProgramfileTab::getAssemblyText() {
    return m_ui->assemblyedit->toPlainText();
}

const QByteArray& ProgramfileTab::getBinaryData() {
    return m_ui->assemblyedit->getCurrentOutputArray();
}

void ProgramfileTab::clearOutputArray() {
    m_ui->assemblyedit->clearOutputArray();
}

ProgramfileTab::~ProgramfileTab() {
    delete m_ui;
}

void ProgramfileTab::setTimerEnabled(bool state) {
    m_ui->assemblyedit->setTimerEnabled(state);
}

void ProgramfileTab::newProgram() {
    m_ui->assemblyfile->toggle();
    m_ui->assemblyedit->reset();
    m_ui->assemblyedit->clear();
}

void ProgramfileTab::on_pushButton_clicked() {
    // load file based on current file type selection
    if (m_ui->binaryfile->isChecked()) {
        emit loadBinaryFile();
    } else {
        loadAssemblyFile();
    }
}

void ProgramfileTab::setAssemblyText(const QString& text) {
    m_ui->assemblyedit->reset();
    m_ui->assemblyedit->setPlainText(text);
}

void ProgramfileTab::setDisassemblerText() {
    const QString& text = m_ui->disassembledViewButton->isChecked() ? Parser::getParser()->getDisassembledRepr()
                                                                    : Parser::getParser()->getBinaryRepr();
    m_ui->binaryedit->setPlainText(text);
    m_ui->binaryedit->updateBreakpoints();
}

void ProgramfileTab::assemblingComplete(const QByteArray& arr, bool clear, uint32_t baseAddress) {
    if (clear)
        Parser::getParser()->clear();
    // Pretty hacky way to discern between the text and data segments
    if (baseAddress > 0) {
        Parser::getParser()->loadFromByteArrayIntoData(arr);
    } else {
        Parser::getParser()->loadFromByteArray(arr, m_ui->disassembledViewButton->isChecked(), baseAddress);
        setDisassemblerText();
    }
    emit updateSimulator();
}

void ProgramfileTab::on_assemblyfile_toggled(bool checked) {
    // Since we are removing the input text/binary info, we need to reset the pipeline
    Pipeline::getPipeline()->reset();

    // handles toggling between assembly input and binary input
    if (checked) {
        m_ui->assemblyedit->setEnabled(true);
        m_ui->assemblyedit->setTimerEnabled(true);
    } else {
        // Disable when loading binary files
        m_ui->assemblyedit->setTimerEnabled(false);
        m_ui->assemblyedit->setEnabled(false);
    }
    // clear both editors when switching input mode and reset the highlighter for the assembly editor
    m_ui->assemblyedit->clear();
    m_ui->assemblyedit->reset();
    m_ui->binaryedit->clear();
}

void ProgramfileTab::setInputMode(bool isAssembly) {
    if (isAssembly) {
        m_ui->assemblyfile->setChecked(true);
    } else {
        m_ui->binaryfile->setChecked(true);
    }
}

void ProgramfileTab::on_disassembledViewButton_toggled(bool checked) {
    Q_UNUSED(checked)
    if (m_ui->binaryfile->isChecked()) {
        assemblingComplete(Parser::getParser()->getFileByteArray());
    } else {
        assemblingComplete(m_ui->assemblyedit->getCurrentOutputArray());
    }
}
