#include "programfiletab.h"
#include "ui_programfiletab.h"

ProgramfileTab::ProgramfileTab(QWidget* parent) : QWidget(parent), m_ui(new Ui::ProgramfileTab) {
    m_ui->setupUi(this);
}

ProgramfileTab::~ProgramfileTab() { delete m_ui; }

void ProgramfileTab::on_pushButton_clicked() {
    // load file based on current file type selection
    if (m_ui->binaryfile->isChecked()) {
        emit loadBinaryFile();
    } else {
        loadAssemblyFile();
    }
}

void ProgramfileTab::setAssemblyText(const QString& text) {
    m_ui->assemblyedit->setPlainText(text);
}

void ProgramfileTab::setBinaryText(const QString& text) { m_ui->binaryedit->setPlainText(text); }
