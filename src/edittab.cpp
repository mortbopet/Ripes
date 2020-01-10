#include "edittab.h"
#include "ui_edittab.h"

#include <QMessageBox>

#include "parser.h"
#include "pipeline.h"

EditTab::EditTab(ProcessorHandler& handler, QToolBar* toolbar, QWidget* parent)
    : RipesTab(toolbar, parent), m_ui(new Ui::EditTab), m_handler(handler) {
    m_ui->setupUi(this);
    m_ui->binaryedit->setHandler(&m_handler);

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

QString EditTab::getAssemblyText() {
    return m_ui->assemblyedit->toPlainText();
}

const QByteArray& EditTab::getBinaryData() {
    return m_assembler->getTextSegment();
}

void EditTab::clear() {
    m_ui->assemblyedit->reset();
    m_assembler->clear();
}

void EditTab::emitProgramChanged() {
    emit programChanged(m_assembler->getProgram());
}

void EditTab::assemble() {
    if (m_ui->assemblyedit->syntaxAccepted()) {
        const QByteArray& ret = m_assembler->assemble(*m_ui->assemblyedit->document());
        if (!m_assembler->hasError()) {
            assemblingComplete(ret, true, 0x0);
            if (m_assembler->hasData()) {
                assemblingComplete(m_assembler->getDataSegment(), false, DATASTART);
            }
            emitProgramChanged();
        } else {
            QMessageBox err;
            err.setText("Error during assembling of program");
            err.exec();
        }
    }

    // Restart the simulator to trigger the data memory to be loaded into the main memory. Bad code that this is done
    // from here, but it works
    Pipeline::getPipeline()->restart();
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

void EditTab::setDisassemblerText() {
    const QString& text = m_ui->disassembledViewButton->isChecked() ? Parser::getParser()->getDisassembledRepr()
                                                                    : Parser::getParser()->getBinaryRepr();
    m_ui->binaryedit->setPlainText(text);
}

void EditTab::assemblingComplete(const QByteArray& data, bool clear, uint32_t baseAddress) {
    if (clear)
        Parser::getParser()->clear();
    // Pretty hacky way to discern between the text and data segments
    if (baseAddress > 0) {
        Parser::getParser()->loadFromByteArrayIntoData(data);
    } else {
        Parser::getParser()->loadFromByteArray(data, m_ui->disassembledViewButton->isChecked(), baseAddress);
        setDisassemblerText();
    }
    emit updateSimulator();
}

void EditTab::on_assemblyfile_toggled(bool checked) {
    // Since we are removing the input text/binary info, we need to reset the pipeline
    Pipeline::getPipeline()->reset();

    if (!checked) {
        // Disable assembly edit when loading binary files
        m_ui->assemblyedit->setEnabled(false);
    }
    // clear both editors when switching input mode and reset the highlighter for the assembly editor
    m_ui->assemblyedit->clear();
    m_ui->assemblyedit->reset();
    m_ui->binaryedit->clear();
}

void EditTab::setInputMode(bool isAssembly) {
    /*
    if (isAssembly) {
        m_ui->assemblyfile->setChecked(true);
    } else {
        m_ui->binaryfile->setChecked(true);
    }
    */
}

void EditTab::on_disassembledViewButton_toggled(bool checked) {
    Q_UNUSED(checked)
    // if (m_ui->binaryfile->isChecked()) {
    //    assemblingComplete(Parser::getParser()->getFileByteArray());
    //} else {
    assemblingComplete(getBinaryData());
    // }
}
