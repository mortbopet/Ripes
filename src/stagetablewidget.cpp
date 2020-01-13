#include "stagetablewidget.h"
#include "ui_stagetablewidget.h"

#include <QClipboard>
#include <QHeaderView>

#include "stagetablemodel.h"

namespace Ripes {

StageTableWidget::StageTableWidget(StageTableModel* model, QWidget* parent)
    : QDialog(parent), m_ui(new Ui::StageTableWidget) {
    m_ui->setupUi(this);

    m_stageModel = model;
    m_ui->stageTableView->setModel(m_stageModel);

    m_ui->stageTableView->horizontalHeader()->setMinimumSectionSize(0);
    m_ui->stageTableView->resizeColumnsToContents();
    m_ui->copy->setIcon(QIcon(":/icons/documents.svg"));
}

StageTableWidget::~StageTableWidget() {
    delete m_ui;
}

void StageTableWidget::on_copy_clicked() {
    // Copy entire table to clipboard, including headers
    Q_ASSERT(m_stageModel != nullptr);
    QString textualRepr;

    // Copy headers
    textualRepr.append('\t');
    for (int j = 0; j < m_stageModel->columnCount(); j++) {
        textualRepr.append(m_stageModel->headerData(j, Qt::Horizontal).toString());
        textualRepr.append('\t');
    }
    textualRepr.append('\n');
    // Copy data
    for (int i = 0; i < m_stageModel->rowCount(); i++) {
        textualRepr.append(m_stageModel->headerData(i, Qt::Vertical).toString());
        textualRepr.append('\t');
        for (int j = 0; j < m_stageModel->columnCount(); j++) {
            textualRepr.append(m_stageModel->data(m_stageModel->index(i, j)).toString());
            textualRepr.append('\t');
        }
        textualRepr.append('\n');
    }
    QApplication::clipboard()->setText(textualRepr);
}
}  // namespace Ripes
