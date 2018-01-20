#ifndef PIPELINETABLEMODEL_H
#define PIPELINETABLEMODEL_H

#include <QAbstractTableModel>

class Pipeline;
class Parser;
class PipelineTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit PipelineTableModel(QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    Pipeline* m_pipelinePtr;
    Parser* m_parserPtr;
};

#endif  // PIPELINETABLEMODEL_H
