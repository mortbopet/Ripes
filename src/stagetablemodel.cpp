#include "stagetablemodel.h"

#include <vector>

namespace Ripes {

static inline uint32_t indexToAddress(unsigned index) {
    if (auto spt = ProcessorHandler::get()->getProgram()) {
        return (index * ProcessorHandler::get()->currentISA()->bytes()) + spt->getSection(TEXT_SECTION_NAME)->address;
    }
    return 0;
}

StageTableModel::StageTableModel(QObject* parent) : QAbstractTableModel(parent) {}

QVariant StageTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        // Cycle number
        return QString::number(section);
    } else {
        const auto addr = indexToAddress(section);
        return ProcessorHandler::get()->disassembleInstr(addr);
    }
}

int StageTableModel::rowCount(const QModelIndex&) const {
    return ProcessorHandler::get()->getCurrentProgramSize() / ProcessorHandler::get()->currentISA()->bytes();
}

int StageTableModel::columnCount(const QModelIndex&) const {
    return m_cycleStageInfos.size();
}

void StageTableModel::processorWasClocked() {
    gatherStageInfo();
}

void StageTableModel::reset() {
    beginResetModel();
    m_cycleStageInfos.clear();
    endResetModel();
}

void StageTableModel::gatherStageInfo() {
    for (unsigned i = 0; i < ProcessorHandler::get()->getProcessor()->stageCount(); i++) {
        m_cycleStageInfos[ProcessorHandler::get()->getProcessor()->getCycleCount()][i] =
            ProcessorHandler::get()->getProcessor()->stageInfo(i);
    }
}

QVariant StageTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (!m_cycleStageInfos.count(index.column()))
        return QVariant();

    const uint32_t addr = indexToAddress(index.row());
    const auto& stageInfo = m_cycleStageInfos.at(index.column());

    QStringList stagesForAddr;
    for (const auto& si : stageInfo) {
        if (si.second.pc == addr && si.second.stage_valid) {
            if (m_cycleStageInfos.count(index.column() - 1)) {
                const auto& prevCycleStageInfo = m_cycleStageInfos.at(index.column() - 1);
                if (prevCycleStageInfo.at(si.first).stage_valid && prevCycleStageInfo.at(si.first).pc == si.second.pc) {
                    stagesForAddr << "-";
                    continue;
                }
            }
            stagesForAddr << ProcessorHandler::get()->getProcessor()->stageName(si.first);
        }
    }

    if (stagesForAddr.size() == 0) {
        return QVariant();
    }

    return stagesForAddr.join('/');
}
}  // namespace Ripes
