#include "instructionmodel.h"
#include "parser.h"
#include "runner.h"

#include <QHeaderView>

InstructionModel::InstructionModel(const StagePCS& pcsptr, Parser* parser, QObject* parent)
    : m_pcsptr(pcsptr), m_parserPtr(parser), QAbstractTableModel(parent) {
    /*
    setHeaderData(0, Qt::Horizontal, "PC");
    setHeaderData(1, Qt::Horizontal, "Stage");
    setHeaderData(2, Qt::Horizontal, "Instruction");
    */
    m_memory = Runner::getRunner()->getMemoryPtr();
}

void InstructionModel::update() {
    // Called when changes to the memory has been made
    // assumes that only instructions are present in the memory when called!
    beginResetModel();
    m_textSize = Runner::getRunner()->getTextSize();
    endResetModel();
}

int InstructionModel::rowCount(const QModelIndex&) const {
    return m_textSize / 4;
}

int InstructionModel::columnCount(const QModelIndex&) const {
    return 3;
}

QVariant InstructionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    int row = index.row();
    switch (index.column()) {
        case 0:
            return row * 4;
        case 1: {  // check if instruction is in any pipeline stage
            int byteIndex = row * 4;
            if (byteIndex == m_pcsptr.EX)
                return QString("EX");
            else if (byteIndex == m_pcsptr.ID)
                return QString("ID");
            else if (byteIndex == m_pcsptr.IF)
                return QString("IF");
            else if (byteIndex == m_pcsptr.MEM)
                return QString("MEM");
            else if (byteIndex == m_pcsptr.WB)
                return QString("WB");
            else
                return QVariant();
        }
        case 2:
            return m_parserPtr->genStringRepr(memRead(row * 4));
        default:
            return QVariant();
    }
}

QVariant InstructionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return "PC";
            case 1:
                return "Stage";
            case 2:
                return "Instruction";
            default:
                return QVariant();
        }
    }

    return QVariant();
}

uint32_t InstructionModel::memRead(uint32_t address) const {
    // Note: If address is not found in memory map, a default constructed object
    // will be created, and read. in our case uint8_t() = 0
    uint32_t read = ((*m_memory)[address] | ((*m_memory)[address + 1] << 8) | ((*m_memory)[address + 2] << 16) |
                     ((*m_memory)[address + 3] << 24));
    return read;
}
