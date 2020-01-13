#pragma once

#include <QDialog>

namespace Ui {
class StageTableWidget;
}

class StageTableModel;

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

class StageTableWidget : public QDialog {
    Q_OBJECT

public:
    StageTableWidget(StageTableModel* model, QWidget* parent = nullptr);
    ~StageTableWidget() override;

private slots:
    void on_copy_clicked();

private:
    Ui::StageTableWidget* m_ui;
    StageTableModel* m_stageModel = nullptr;
};
