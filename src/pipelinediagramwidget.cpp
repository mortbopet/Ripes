#include "pipelinediagramwidget.h"
#include "ui_pipelinediagramwidget.h"

#include <QClipboard>
#include <QHeaderView>

#include "pipelinediagrammodel.h"
#include "ripessettings.h"

namespace Ripes {

PipelineDiagramWidget::PipelineDiagramWidget(PipelineDiagramModel* model, QWidget* parent)
    : QDialog(parent), m_ui(new Ui::PipelineDiagramWidget) {
    m_ui->setupUi(this);

    m_stageModel = model;
    m_ui->pipelineDiagramView->setModel(m_stageModel);

    m_ui->pipelineDiagramView->resizeColumnsToContents();
    m_ui->copy->setIcon(QIcon(":/icons/documents.svg"));

    m_stageModel->prepareForView();
}

PipelineDiagramWidget::~PipelineDiagramWidget() {
    delete m_ui;
}

void PipelineDiagramWidget::on_copy_clicked() {
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
    for (int i = 0; i < m_stageModel->rowCount(); ++i) {
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
