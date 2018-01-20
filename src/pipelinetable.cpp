#include "pipelinetable.h"
#include "ui_pipelinetable.h"

#include <QHeaderView>

PipelineTable::PipelineTable(QWidget* parent) : QDialog(parent), ui(new Ui::PipelineTable) {
    ui->setupUi(this);
    this->setModal(true);
    ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void PipelineTable::setModel(QAbstractItemModel* model) {
    ui->table->setModel(model);
}

PipelineTable::~PipelineTable() {
    delete ui;
}
