#include "instructionmodel.h"
#include <QHeaderView>

#include "processorhandler.h"

namespace Ripes {
AInt InstructionModel::indexToAddress(const QModelIndex& index) const {
    if (ProcessorHandler::currentISA()->extensionEnabled("C")) {
        if (auto prog_spt = ProcessorHandler::getProgram()) {
            AInt addr = 0;
            for (int rowC = 0; rowC < index.row(); rowC++) {
                const VInt instr = ProcessorHandler::getMemory().readMem(addr, 2);
                if ((instr & 0b11) == 0b11)  // 32bits
                {
                    addr += 2;
                }
                addr += 2;
            }
            return addr + prog_spt->getSection(TEXT_SECTION_NAME)->address;
        }
    } else {
        if (auto prog_spt = ProcessorHandler::getProgram()) {
            return (index.row() * 4) + prog_spt->getSection(TEXT_SECTION_NAME)->address;
        }
    }
    return 0;
}

int InstructionModel::addressToRow(AInt addr) const {
    if (ProcessorHandler::currentISA()->extensionEnabled("C")) {
        if (addr == 0)
            return 0;
        if (auto prog_spt = ProcessorHandler::getProgram()) {
            int rowC = 0;
            for (AInt addrC = 0; addrC < (addr - prog_spt->getSection(TEXT_SECTION_NAME)->address); addrC += 2) {
                const VInt word = ProcessorHandler::getMemory().readMem(addrC, 2);
                if ((word & 0b11) == 0b11)  // 32bits
                {
                    addrC += 2;
                }
                rowC++;
            }
            return rowC;
        }
    } else {
        if (auto prog_spt = ProcessorHandler::getProgram()) {
            if (prog_spt->getSection(TEXT_SECTION_NAME) != nullptr) {
                return (addr - prog_spt->getSection(TEXT_SECTION_NAME)->address) / 4;
            }
        }
    }
    return 0;
}

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
    if (ProcessorHandler::currentISA()->extensionEnabled("C")) {
        m_rowCount = 0;
        for (int addr = 0; addr < ProcessorHandler::getCurrentProgramSize(); addr += 2) {
            const VInt word = ProcessorHandler::getMemory().readMem(addr, 16);
            if ((word & 0x11) == 0x11)  // 32bits
            {
                addr += 2;
            }
            m_rowCount++;
        }
    } else {
        m_rowCount = ProcessorHandler::getCurrentProgramSize() / ProcessorHandler::currentISA()->instrBytes();
    }
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
            const AInt oldAddress = oldStageInfo.pc;
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
    const AInt addr = indexToAddress(index);
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

QVariant InstructionModel::BPData(AInt addr) const {
    return ProcessorHandler::hasBreakpoint(addr);
}
QVariant InstructionModel::PCData(AInt addr) const {
    return "0x" + QString::number(addr, 16);
}
QVariant InstructionModel::stageData(AInt addr) const {
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

QVariant InstructionModel::instructionData(AInt addr) const {
    return ProcessorHandler::disassembleInstr(addr);
}

QVariant InstructionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    const AInt addr = indexToAddress(index);
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
