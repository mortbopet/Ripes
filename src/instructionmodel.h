#pragma once

#include <QColor>
#include <set>
#include "processorhandler.h"

#include <QAbstractTableModel>

namespace Ripes {

class Parser;
class Pipeline;

static inline AInt indexToAddress(const QModelIndex& index) {
    if (auto prog_spt = ProcessorHandler::getProgram()) {
        return (index.row() * 4) + prog_spt->getSection(TEXT_SECTION_NAME)->address;
    }
    return 0;
}

static inline int addressToRow(AInt addr) {
    if (auto prog_spt = ProcessorHandler::getProgram()) {
        if (prog_spt->getSection(TEXT_SECTION_NAME) != nullptr) {
            return (addr - prog_spt->getSection(TEXT_SECTION_NAME)->address) / 4;
        }
    }
    return 0;
}

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

signals:
    /**
     * @brief firstStageInstrChanged
     * Emitted whenever the PC of the first stage, changed. Argument is the address of the instruction now present in
     * the first stage.
     */
    void firstStageInstrChanged(AInt);

private:
    void updateStageInfo();

    QVariant BPData(AInt addr) const;
    QVariant PCData(AInt addr) const;
    QVariant stageData(AInt addr) const;
    QVariant instructionData(AInt addr) const;
    void updateRowCount();
    void onProcessorReset();

    QStringList m_stageNames;
    using StageID = unsigned;
    std::map<StageID, StageInfo> m_stageInfos;
    int m_rowCount = 0;
};
}  // namespace Ripes
