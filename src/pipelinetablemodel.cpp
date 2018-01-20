#include "pipelinetablemodel.h"
#include "pipeline.h"

#include "parser.h"

#include <vector>

PipelineTableModel::PipelineTableModel(QObject* parent) : QAbstractTableModel(parent) {
    m_pipelinePtr = Pipeline::getPipeline();
    m_parserPtr = Parser::getParser();
}

QVariant PipelineTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        // Cycle number
        return QString::number(section);
    } else {
        return m_parserPtr->getInstructionString(section * 4);
    }
    return QVariant();
}
int PipelineTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    int size = m_pipelinePtr->getTextSize() / 4;
    return size;
}

int PipelineTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    int size = m_pipelinePtr->getPcsList().size();
    return size;
}

QVariant PipelineTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    int cycle = index.column();
    uint32_t addr = index.row() * 4;
    const StagePCS& pcs = m_pipelinePtr->getPcsList()[cycle];
    StagePCS pcsPre;
    if (cycle > 0)
        pcsPre = m_pipelinePtr->getPcsList()[cycle - 1];

    QString stage;
    if (pcs.IF.pc == addr && pcs.IF.initialized == true && pcs.IF.invalidReason == 0) {
        // Check if PC is duplicate in current and previous stage. If so, a stall is present, and we write a "-" in the
        // table
        if (pcs.IF.pc != pcsPre.IF.pc) {
            stage = "F";
        } else {
            stage = "-";
        }
    } else if (pcs.ID.pc == addr && pcs.ID.initialized == true && pcs.ID.invalidReason == 0) {
        if ((pcs.ID.pc != pcsPre.ID.pc) || !pcsPre.ID.initialized) {
            stage = "D";
        } else {
            stage = "-";
        }
    } else if (pcs.EX.pc == addr && pcs.EX.initialized == true && pcs.EX.invalidReason == 0) {
        stage = "E";
    } else if (pcs.MEM.pc == addr && pcs.MEM.initialized == true && pcs.MEM.invalidReason == 0) {
        stage = "M";
    } else if (pcs.WB.pc == addr && pcs.WB.initialized == true && pcs.WB.invalidReason == 0) {
        stage = "W";
    }
    return stage;

    return QVariant();
}
