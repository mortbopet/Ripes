#pragma once

#include <QAbstractTableModel>

#include "processorhandler.h"

namespace Ripes {

class StageTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Breakpoint = 0, PC = 1, Stage = 2, Instruction = 3, NColumns };
    StageTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    void processorWasClocked();
    void reset();

private:
    void gatherStageInfo();

    std::map<unsigned, std::map<QString, StageInfo>> m_cycleStageInfos;
};
}  // namespace Ripes
