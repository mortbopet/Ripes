#pragma once

#include <set>

#include <QAbstractTableModel>
#include <QColor>

#include "assembler/program.h"
#include "processors/interface/ripesprocessor.h"

namespace Ripes {

class Parser;
class Pipeline;

class InstructionModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column { Breakpoint = 0, PC = 1, Stage = 2, Instruction = 3, NColumns };
    InstructionModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    AInt indexToAddress(const QModelIndex& index) const;
    int addressToRow(AInt addr) const;

signals:
    /**
     * @brief firstStageInstrChanged
     * Emitted whenever the PC of the first stage, changed. Argument is the row of the instruction now present
     * in the first stage.
     */
    void firstStageInstrChanged(int row);

private:
    void updateStageInfo();

    QVariant BPData(AInt addr) const;
    QVariant PCData(AInt addr) const;
    QVariant stageData(AInt addr) const;
    QVariant instructionData(AInt addr) const;
    void updateRowCount();
    void onProcessorReset();

    std::shared_ptr<const Program> m_program;
    QStringList m_stageNames;
    using StageID = unsigned;
    std::map<StageID, StageInfo> m_stageInfos;
    int m_rowCount = 0;
};
}  // namespace Ripes
