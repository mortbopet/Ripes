#include "instructionmodel.h"

#include <QHeaderView>

namespace Ripes {

InstructionModel::InstructionModel(QObject* parent) : QAbstractTableModel(parent) {
    for (unsigned i = 0; i < ProcessorHandler::getProcessor()->stageCount(); i++) {
        m_stageNames << ProcessorHandler::getProcessor()->stageName(i);
        m_stageInfos[i];
    }
    connect(ProcessorHandler::get(), &ProcessorHandler::procStateChangedNonRun, this,
            &InstructionModel::updateStageInfo);
    connect(ProcessorHandler::get(), &ProcessorHandler::processorReset, this, &InstructionModel::onProcessorReset);
    onProcessorReset();
}

void InstructionModel::onProcessorReset() {
    updateRowCount();
    beginResetModel();
    endResetModel();
    updateStageInfo();
}

int InstructionModel::columnCount(const QModelIndex&) const {
    return NColumns;
}

void InstructionModel::updateRowCount() {
    m_rowCount = ProcessorHandler::getCurrentProgramSize() / ProcessorHandler::currentISA()->bytes();
}

int InstructionModel::rowCount(const QModelIndex&) const {
    return m_rowCount;
}

void InstructionModel::updateStageInfo() {
    bool firstStageChanged = false;
    for (unsigned i = 0; i < ProcessorHandler::getProcessor()->stageCount(); i++) {
        if (static_cast<unsigned>(m_stageInfos.size()) > i) {
            auto& oldStageInfo = m_stageInfos.at(i);
            if (i == 0) {
                if (oldStageInfo.pc != ProcessorHandler::getProcessor()->stageInfo(i).pc) {
                    firstStageChanged = true;
                }
            }
            const auto stageInfo = ProcessorHandler::getProcessor()->stageInfo(i);
            const uint32_t oldAddress = oldStageInfo.pc;
            const bool stageInfoChanged = oldStageInfo != stageInfo;
            oldStageInfo = stageInfo;
            if (stageInfoChanged) {
                const int oldRow = addressToRow(oldAddress);
                const int newRow = addressToRow(stageInfo.pc);
                const QModelIndex oldIdx = index(oldRow, Stage);
                const QModelIndex newIdx = index(newRow, Stage);
                emit dataChanged(oldIdx, oldIdx, {Qt::DisplayRole});
                emit dataChanged(newIdx, newIdx, {Qt::DisplayRole});
            }
            if (firstStageChanged) {
                emit firstStageInstrChanged(m_stageInfos.at(0).pc);
                firstStageChanged = false;
            }
        }
    }
}

bool InstructionModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    const uint32_t addr = indexToAddress(index);
    if ((index.column() == Column::Breakpoint) && role == Qt::CheckStateRole) {
        if (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked) {
            ProcessorHandler::setBreakpoint(addr, !ProcessorHandler::hasBreakpoint(addr));
            return true;
        }
    }
    return false;
}

QVariant InstructionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && (role == Qt::DisplayRole || role == Qt::ToolTipRole)) {
        switch (section) {
            case Column::Breakpoint:
                return role == Qt::DisplayRole ? "BP" : "Breakpoints";
            case Column::PC:
                return role == Qt::DisplayRole ? "Addr" : "Instruction Address";
            case Column::Stage:
                return role == Qt::DisplayRole ? "Stage" : "Stages currently executing instructon";
            case Column::Instruction:
                return "Instruction";
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant InstructionModel::BPData(uint32_t addr) const {
    return ProcessorHandler::hasBreakpoint(addr);
}
QVariant InstructionModel::PCData(uint32_t addr) const {
    return "0x" + QString::number(addr, 16);
}
QVariant InstructionModel::stageData(uint32_t addr) const {
    QStringList stagesForAddr;
    for (const auto& si : m_stageInfos) {
        if ((si.second.pc == addr) && si.second.stage_valid) {
            stagesForAddr << m_stageNames.at(si.first);
        }
    }
    if (stagesForAddr.isEmpty()) {
        return QVariant();
    } else {
        return stagesForAddr.join("/");
    }
}

QVariant InstructionModel::instructionData(uint32_t addr) const {
    return ProcessorHandler::disassembleInstr(addr);
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
}  // namespace Ripes
