#pragma once

#include <QAbstractTableModel>
#include "processors/interface/ripesprocessor.h"

namespace Ripes {

class PipelineDiagramModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Breakpoint = 0, PC = 1, Stage = 2, Instruction = 3, NColumns };
    PipelineDiagramModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void prepareForView();

public slots:
    void processorWasClocked();
    void reset();

private:
    void gatherStageInfo();

    /**
     * @brief m_cycleStageInfos
     * map<cycle, map<stageId, stageInfo>>
     */
    std::map<long long, std::map<unsigned, StageInfo>> m_cycleStageInfos;

    /**
     * @brief m_atMaxCycles
     * Records the state that we've crossed the threshold set for RIPES_SETTING_PIPEDIAGRAM_MAXCYCLES.
     * This is to speed up execution by not having to check the settings value during each cycle of simulation,
     * after the value has been reached.
     */
    bool m_atMaxCycles = false;
};
}  // namespace Ripes
