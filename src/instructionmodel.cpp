#include "instructionmodel.h"
#include "parser.h"
#include "pipeline.h"

#include <QHeaderView>

namespace {
uint32_t indexToAddress(const QModelIndex& index) {
    return index.row() * 4;
}
}  // namespace

InstructionModel::InstructionModel(ProcessorHandler& handler, QObject* parent)
    : QAbstractTableModel(parent), m_handler(handler) {
    for (int i = 0; i < m_handler.getProcessor()->stageCount(); i++) {
        m_stageNames << m_handler.getProcessor()->stageName(i);
        m_stageInfos[m_stageNames.last()];
    }
}

int InstructionModel::columnCount(const QModelIndex&) const {
    return NColumns;
}

int InstructionModel::rowCount(const QModelIndex&) const {
    // Each instruction is 4 bytes and each row represents an instruction within the .text segment
    return Pipeline::getPipeline()->getTextSize() / 4;
}

void InstructionModel::processorWasClocked() {
    // Reload model
    beginResetModel();
    gatherStageInfo();
    endResetModel();
}

void InstructionModel::gatherStageInfo() {
    bool firstStageChanged = false;
    for (int i = 0; i < m_stageNames.length(); i++) {
        if (i == 0) {
            if (m_stageInfos[m_stageNames[i]].pc != m_handler.getProcessor()->stageInfo(i).pc) {
                firstStageChanged = true;
            }
        }
        m_stageInfos[m_stageNames[i]] = m_handler.getProcessor()->stageInfo(i);
        if (firstStageChanged) {
            emit firstStageInstrChanged(m_stageInfos[m_stageNames[0]].pc);
            firstStageChanged = false;
        }
    }
}

bool InstructionModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    const uint32_t addr = indexToAddress(index);
    if ((index.column() == Column::Breakpoint) && role == Qt::CheckStateRole) {
        if (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked) {
            m_handler.setBreakpoint(addr, !m_handler.hasBreakpoint(addr));
            return true;
        }
    }
    return false;
}

QVariant InstructionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case Column::Breakpoint:
                return "BP";
            case Column::PC:
                return "PC";
            case Column::Stage:
                return "Stage";
            case Column::Instruction:
                return "Instruction";
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant InstructionModel::BPData(uint32_t addr) const {
    return m_handler.hasBreakpoint(addr);
}
QVariant InstructionModel::PCData(uint32_t addr) const {
    return addr;
}
QVariant InstructionModel::stageData(uint32_t addr) const {
    for (const auto& si : m_stageInfos) {
        if ((si.second.pc == addr) && si.second.pc_valid) {
            return si.first;
        }
    }
    return QVariant();
}

QVariant InstructionModel::instructionData(uint32_t addr) const {
    return m_handler.parseInstrAt(addr);
}

QVariant InstructionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    const uint32_t addr = indexToAddress(index);
    switch (index.column()) {
        case Column::Breakpoint: {
            if (role == Qt::CheckStateRole) {
                return BPData(addr);
            }
            break;
        }
        case Column::PC: {
            if (role == Qt::DisplayRole) {
                return PCData(addr);
            }
            break;
        }
        case Column::Stage: {
            if (role == Qt::DisplayRole) {
                return stageData(addr);
            }
            break;
        }
        case Column::Instruction: {
            if (role == Qt::DisplayRole) {
                return instructionData(addr);
            }
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags InstructionModel::flags(const QModelIndex& index) const {
    const auto def = Qt::ItemIsEnabled;
    if (index.column() == Column::Breakpoint)
        return Qt::ItemIsUserCheckable | def;
    return def;
}
