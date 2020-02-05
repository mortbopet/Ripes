#pragma once

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

namespace Ripes {
class StageTableModel;
namespace Ui {
class StageTableWidget;
}

class StageTableWidget : public QDialog {
    Q_OBJECT

public:
    StageTableWidget(StageTableModel* model, QWidget* parent = nullptr);
    ~StageTableWidget() override;

private slots:
    void on_copy_clicked();

private:
    Ui::StageTableWidget* m_ui = nullptr;
    StageTableModel* m_stageModel = nullptr;
};
}  // namespace Ripes
