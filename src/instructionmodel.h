#pragma once

#include <QColor>
#include <set>
#include "defines.h"
#include "mainmemory.h"
#include "processorhandler.h"

#include <QAbstractTableModel>

namespace Ripes {

class Parser;
class Pipeline;

static inline uint32_t indexToAddress(const QModelIndex& index) {
    if (auto prog_spt = ProcessorHandler::get()->getProgram()) {
        return (index.row() * 4) + prog_spt->getSection(TEXT_SECTION_NAME)->address;
    }
    return 0;
}

static inline int addressToIndex(uint32_t addr) {
    if (auto prog_spt = ProcessorHandler::get()->getProgram()) {
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

public slots:
    void processorWasClocked();

signals:
    /**
     * @brief firstStageInstrChanged
     * Emitted whenever the PC of the first stage, changed. Argument is the address of the instruction now present in
     * the first stage.
     */
    void firstStageInstrChanged(uint32_t) const;

private:
    void gatherStageInfo();

    QVariant BPData(uint32_t addr) const;
    QVariant PCData(uint32_t addr) const;
    QVariant stageData(uint32_t addr) const;
    QVariant instructionData(uint32_t addr) const;

    QStringList m_stageNames;
    std::map<QString, StageInfo> m_stageInfos;
};
}  // namespace Ripes
