#ifndef PIPELINETABLE_H
#define PIPELINETABLE_H

#include <QDialog>

namespace Ui {
class PipelineTable;
}

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

class PipelineTable : public QDialog {
    Q_OBJECT

public:
    explicit PipelineTable(QWidget* parent = 0);
    ~PipelineTable();
    void setModel(QAbstractItemModel* model);

private slots:
    void on_save_clicked();

    void on_copy_clicked();

private:
    Ui::PipelineTable* ui;
    QAbstractItemModel* m_model = nullptr;
};

#endif  // PIPELINETABLE_H
