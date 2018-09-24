#include "pipelinetable.h"
#include "ui_pipelinetable.h"

#include <QClipboard>
#include <QFileDialog>
#include <QHeaderView>

PipelineTable::PipelineTable(QWidget* parent) : QDialog(parent), ui(new Ui::PipelineTable) {
    ui->setupUi(this);
    this->setModal(true);
    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->description->setText(QString("<b>Note:</b> The pipeline table can be copied as tab separated,<br>suitable for pasting into a spreadsheet"));
}

void PipelineTable::setModel(QAbstractItemModel* model) {
    ui->table->setModel(model);
    m_model = model;
}

PipelineTable::~PipelineTable() {
    delete ui;
}

void PipelineTable::on_copy_clicked() {
    // Copy entire table to clipboard, including headers
    Q_ASSERT(m_model != nullptr);
    QString textualRepr;

    // Copy headers
    textualRepr.append('\t');
    for (int j = 0; j < m_model->columnCount(); j++) {
        textualRepr.append(m_model->headerData(j, Qt::Horizontal).toString());
        textualRepr.append('\t');
    }
    textualRepr.append('\n');
    // Copy data
    for (int i = 0; i < m_model->rowCount(); i++) {
        textualRepr.append(m_model->headerData(i, Qt::Vertical).toString());
        textualRepr.append('\t');
        for (int j = 0; j < m_model->columnCount(); j++) {
            textualRepr.append(m_model->data(m_model->index(i, j)).toString());
            textualRepr.append('\t');
        }
        textualRepr.append('\n');
    }
    QApplication::clipboard()->setText(textualRepr);
}
