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

private:
    Ui::PipelineTable* ui;
};

#endif  // PIPELINETABLE_H
