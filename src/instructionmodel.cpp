#include "instructionmodel.h"
#include "parser.h"

#include <QHeaderView>

InstructionModel::InstructionModel(Parser* parser, QObject* parent) : m_parserPtr(parser), QAbstractTableModel(parent) {
    /*
    setHeaderData(0, Qt::Horizontal, "PC");
    setHeaderData(1, Qt::Horizontal, "Stage");
    setHeaderData(2, Qt::Horizontal, "Instruction");
    */
}

void InstructionModel::setMemory(memory* mem, int textSize) {
    beginResetModel();
    m_memory = mem;
    m_textSize = textSize;
    endResetModel();
}

void InstructionModel::setTextSize(int textSize) {
    beginResetModel();
    m_textSize = textSize;
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
    switch (index.column()) {
        case 0:
            return index.row() * 4;
        case 1:
            return "NaN";
        case 2:
            return m_parserPtr->genStringRepr(memRead(index.row() * 4));
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
