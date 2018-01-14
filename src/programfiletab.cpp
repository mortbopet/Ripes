#include "programfiletab.h"
#include "ui_programfiletab.h"

ProgramfileTab::ProgramfileTab(QWidget* parent) : QWidget(parent), m_ui(new Ui::ProgramfileTab) {
    m_ui->setupUi(this);

    // Only add syntax highlighter for code edit view - not for translated code. This is assumed to be correct after a
    // translation is complete
    m_ui->assemblyedit->setupSyntaxHighlighter();
    m_ui->binaryedit->setReadOnly(true);
    // enable breakpoint area for the translated code only
    m_ui->binaryedit->enableBreakpointArea();

    // Link scrollbars together for pleasant navigation
    connect(m_ui->assemblyedit->verticalScrollBar(), &QScrollBar::valueChanged, m_ui->binaryedit->verticalScrollBar(),
            &QScrollBar::setValue);
    connect(m_ui->binaryedit->verticalScrollBar(), &QScrollBar::valueChanged, m_ui->assemblyedit->verticalScrollBar(),
            &QScrollBar::setValue);
}

ProgramfileTab::~ProgramfileTab() {
    delete m_ui;
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

void ProgramfileTab::setDisassemblerText(const QString& text) {
    m_ui->binaryedit->clearBreakpoints();
    m_ui->binaryedit->setPlainText(text);
}

void ProgramfileTab::on_assemblyfile_toggled(bool checked) {
    // handles toggling between assembly input and binary input
    if (checked) {
        m_ui->assemblyedit->setEnabled(true);
    } else {
        // Disable when loading binary files
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
