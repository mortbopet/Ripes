#ifndef RWJUMPMODEL_H
#define RWJUMPMODEL_H

#include <QAbstractTableModel>

class Pipeline;
class Parser;
class RWJumpModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit RWJumpModel(QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void update();

private:
    Pipeline* m_pipelinePtr;
    Parser* m_parserPtr;
};

#endif  // RWJUMPMODEL_H
