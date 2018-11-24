#include "rwjumpmodel.h"

#include "parser.h"
#include "pipeline.h"

RWJumpModel::RWJumpModel(QObject* parent) : QAbstractTableModel(parent) {
    m_pipelinePtr = Pipeline::getPipeline();
    m_parserPtr = Parser::getParser();
}

QVariant RWJumpModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return "Cycle";
            case 1:
                return "PC";
            case 2:
                return "Instruction";
            case 3:
                return "Access address";
            default:
                return QVariant();
        }
    }

    return QVariant();
}

int RWJumpModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    int size = m_pipelinePtr->m_MemoryAccesses.size();
    return size;
}

void RWJumpModel::update() {
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
    beginResetModel();
    endResetModel();
}

int RWJumpModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return 4;  // PC, Write type and instrution
}

QVariant RWJumpModel::data(const QModelIndex& index, int role) const {
    const RVAccess& access = m_pipelinePtr->m_MemoryAccesses[index.row()];
    if (role == Qt::UserRole) {
        // Address is requested for jumping
        return access.addr;
    }
    if (role != Qt::DisplayRole)
        return QVariant();
    switch (index.column()) {
        case 0:
            return access.cycle - 1;
        case 1:
            return access.pc;
        case 2: {
            return m_parserPtr->getInstructionString(access.pc);
        }
        case 3: {
            return QString("0x%1").arg(QString::number(access.addr, 16));
        }
    }
    return QVariant();
}
