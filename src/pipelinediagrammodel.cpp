#include "pipelinediagrammodel.h"

#include "ripessettings.h"

#include <vector>

namespace Ripes {

static inline uint32_t indexToAddress(unsigned index) {
    if (auto spt = ProcessorHandler::getProgram()) {
        return (index * ProcessorHandler::currentISA()->bytes()) + spt->getSection(TEXT_SECTION_NAME)->address;
    }
    return 0;
}

PipelineDiagramModel::PipelineDiagramModel(QObject* parent) : QAbstractTableModel(parent) {
    connect(ProcessorHandler::get(), &ProcessorHandler::processorClocked, this,
            &PipelineDiagramModel::processorWasClocked, Qt::DirectConnection);
    connect(ProcessorHandler::get(), &ProcessorHandler::processorReset, this, &PipelineDiagramModel::reset);
}

QVariant PipelineDiagramModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        // Cycle number
        return QString::number(section);
    } else {
        const auto addr = indexToAddress(section);
        return ProcessorHandler::disassembleInstr(addr);
    }
}

int PipelineDiagramModel::rowCount(const QModelIndex&) const {
    return ProcessorHandler::getCurrentProgramSize() / ProcessorHandler::currentISA()->bytes();
}

int PipelineDiagramModel::columnCount(const QModelIndex&) const {
    return m_cycleStageInfos.size();
}

void PipelineDiagramModel::processorWasClocked() {
    if (m_atMaxCycles) {
        return;
    }
    gatherStageInfo();

    const auto cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    if (cycleCount >= RipesSettings::value(RIPES_SETTING_PIPEDIAGRAM_MAXCYCLES).toInt()) {
        m_atMaxCycles = true;
    }
}

void PipelineDiagramModel::reset() {
    m_atMaxCycles = false;
    m_cycleStageInfos.clear();
    gatherStageInfo();
}

void PipelineDiagramModel::prepareForView() {
    beginResetModel();
    endResetModel();
}

void PipelineDiagramModel::gatherStageInfo() {
    long long cycleCount = ProcessorHandler::getProcessor()->getCycleCount();
    if (m_cycleStageInfos.count(cycleCount)) {
        return;
    }
    for (unsigned i = 0; i < ProcessorHandler::getProcessor()->stageCount(); i++) {
        m_cycleStageInfos[ProcessorHandler::getProcessor()->getCycleCount()][i] =
            ProcessorHandler::getProcessor()->stageInfo(i);
    }
}

QVariant PipelineDiagramModel::data(const QModelIndex& index, int role) const {
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
        if (si.second.pc == addr && si.second.stage_valid && si.second.state == StageInfo::State::None) {
            if (m_cycleStageInfos.count(index.column() - 1)) {
                const auto& prevCycleStageInfo = m_cycleStageInfos.at(index.column() - 1);
                if (prevCycleStageInfo.at(si.first).stage_valid && prevCycleStageInfo.at(si.first).pc == si.second.pc) {
                    stagesForAddr << "-";
                    continue;
                }
            }
            stagesForAddr << ProcessorHandler::getProcessor()->stageName(si.first);
        }
    }

    if (stagesForAddr.size() == 0) {
        return QVariant();
    }

    return stagesForAddr.join('/');
}
}  // namespace Ripes
