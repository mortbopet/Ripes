#include "instructionmodel.h"
#include "parser.h"
#include "pipeline.h"

#include <QHeaderView>

InstructionModel::InstructionModel(const StagePCS& pcsptr, const StagePCS& pcsptrPre, Parser* parser, QObject* parent)
    : m_pcsptr(pcsptr), m_pcsptrPre(pcsptrPre), m_parserPtr(parser), QAbstractTableModel(parent) {
    m_pipelinePtr = Pipeline::getPipeline();
    m_memory = m_pipelinePtr->getMemoryPtr();
}

void InstructionModel::update() {
    // Called when changes to the memory has been made
    // assumes that only instructions are present in the memory when called!
    beginResetModel();
    m_textSize = Pipeline::getPipeline()->getTextSize();
    endResetModel();
}

int InstructionModel::rowCount(const QModelIndex&) const {
    return m_textSize / 4;
}

int InstructionModel::columnCount(const QModelIndex&) const {
    return 3;
}

namespace {
#define VALIDATE(stage)                     \
    if (!m_pcsptr.stage.second) {           \
        emit textChanged(Stage::stage, ""); \
    }
}

QVariant InstructionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    int row = index.row();
    switch (index.column()) {
        case 0:
            return row * 4;
        case 1: {  // check if instruction is in any pipeline stage, and whether the given PC for the pipeline stage is
                   // valid. For the pipelinewidget, determine if an instruction in a given stage is equal to the text
                   // size, if so, clear previous instruction texts in the pipeline widget
            uint32_t byteIndex = row * 4;
            uint32_t maxInstr = m_textSize - 4;
            if (byteIndex == m_pcsptr.EX.first && m_pcsptr.EX.second) {
                emit textChanged(Stage::EX, m_parserPtr->genStringRepr(memRead(row * 4)));
                if (byteIndex == maxInstr) {
                    emit textChanged(Stage::ID, "");
                }
                return QString("EX");
            } else if (byteIndex == m_pcsptr.ID.first && m_pcsptr.ID.second) {
                emit textChanged(Stage::ID, m_parserPtr->genStringRepr(memRead(row * 4)));
                if (byteIndex == maxInstr) {
                    emit textChanged(Stage::IF, "");
                }
                return QString("ID");
            } else if (byteIndex == m_pcsptr.IF.first && m_pcsptr.IF.second) {
                emit textChanged(Stage::IF, m_parserPtr->genStringRepr(memRead(row * 4)));
                return QString("IF");
            } else if (byteIndex == m_pcsptr.MEM.first && m_pcsptr.MEM.second) {
                emit textChanged(Stage::MEM, m_parserPtr->genStringRepr(memRead(row * 4)));
                if (byteIndex == maxInstr) {
                    emit textChanged(Stage::EX, "");
                }
                return QString("MEM");
            } else if (byteIndex == m_pcsptr.WB.first && m_pcsptr.WB.second) {
                emit textChanged(Stage::WB, m_parserPtr->genStringRepr(memRead(row * 4)));
                if (byteIndex == maxInstr) {
                    emit textChanged(Stage::MEM, "");
                }
                return QString("WB");
            } else {
                // Clear invalid PC values for each stage (used when resetting the simlation)
                VALIDATE(IF);
                VALIDATE(ID);
                VALIDATE(EX);
                VALIDATE(MEM);
                VALIDATE(WB);
                return QVariant();
            }
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
    uint32_t read = (m_memory->read(address) | (m_memory->read(address + 1) << 8) |
                     (m_memory->read(address + 2) << 16) | (m_memory->read(address + 3) << 24));
    return read;
}
