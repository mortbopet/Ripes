#include "stagetablemodel.h"

#include "parser.h"

#include <vector>

StageTableModel::StageTableModel(QObject* parent) : QAbstractTableModel(parent) {}

QVariant StageTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        // Cycle number
        return QString::number(section);
    } else {
        return ProcessorHandler::get()->parseInstrAt(section *
                                                     ProcessorHandler::get()->getProcessor()->implementsISA()->bytes());
    }
}

int StageTableModel::rowCount(const QModelIndex&) const {
    return ProcessorHandler::get()->getCurrentProgramSize() /
           ProcessorHandler::get()->getProcessor()->implementsISA()->bytes();
}

int StageTableModel::columnCount(const QModelIndex&) const {
    return m_cycleStageInfos.size();
}

void StageTableModel::processorWasClocked() {
    beginResetModel();
    gatherStageInfo();
    endResetModel();
}

void StageTableModel::reset() {
    beginResetModel();
    m_cycleStageInfos.clear();
    endResetModel();
}

void StageTableModel::gatherStageInfo() {
    for (int i = 0; i < ProcessorHandler::get()->getProcessor()->stageCount(); i++) {
        const auto& stageName = ProcessorHandler::get()->getProcessor()->stageName(i);
        m_cycleStageInfos[ProcessorHandler::get()->getProcessor()->getCycleCount()][stageName] =
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

    const uint32_t addr = index.row() * ProcessorHandler::get()->getProcessor()->implementsISA()->bytes();
    const auto& stageInfo = m_cycleStageInfos.at(index.column());
    for (const auto& si : stageInfo) {
        if (si.second.pc == addr && si.second.pc_valid) {
            return si.first;
        }
    }

    return QVariant();
}
