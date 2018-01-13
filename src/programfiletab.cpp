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
