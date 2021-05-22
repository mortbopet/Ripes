#pragma once

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

namespace Ripes {
class PipelineDiagramModel;
namespace Ui {
class PipelineDiagramWidget;
}

class PipelineDiagramWidget : public QDialog {
    Q_OBJECT

public:
    PipelineDiagramWidget(PipelineDiagramModel* model, QWidget* parent = nullptr);
    ~PipelineDiagramWidget() override;

private slots:
    void on_copy_clicked();

private:
    Ui::PipelineDiagramWidget* m_ui = nullptr;
    PipelineDiagramModel* m_stageModel = nullptr;
};
}  // namespace Ripes
