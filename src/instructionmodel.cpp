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
#define VALIDATE(stage)                                       \
    if (!m_pcsptr.stage.initialized) {                        \
        emit textChanged(Stage::stage, "");                   \
    } else if (m_pcsptr.stage.invalid) {                      \
        emit textChanged(Stage::stage, "nop (branch taken)"); \
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
        case 1: {
            // check if instruction is in any pipeline stage, and whether the given PC for the pipeline stage is
            // valid.
            // Furthermore, because of branching, an instruction can be in multiple stages at once - therefore, we build
            // up a return string by checking all stage program counters
            QStringList retStrings;
            uint32_t byteIndex = row * 4;
            uint32_t maxInstr = m_textSize - 4;
            if (byteIndex == m_pcsptr.EX.pc && m_pcsptr.EX.initialized && !m_pcsptr.EX.invalid) {
                emit textChanged(Stage::EX, m_parserPtr->genStringRepr(memRead(row * 4)));
                if (byteIndex == maxInstr) {
                    emit textChanged(Stage::ID, "");
                }
                retStrings << "EX";
            }
            if (byteIndex == m_pcsptr.ID.pc && m_pcsptr.ID.initialized && !m_pcsptr.ID.invalid) {
                emit textChanged(Stage::ID, m_parserPtr->genStringRepr(memRead(row * 4)));
                if (byteIndex == maxInstr) {
                    emit textChanged(Stage::IF, "");
                }
                retStrings << "ID";
            }
            if (byteIndex == m_pcsptr.IF.pc && m_pcsptr.IF.initialized && !m_pcsptr.IF.invalid) {
                emit textChanged(Stage::IF, m_parserPtr->genStringRepr(memRead(row * 4)));
                retStrings << "IF";
            }
            if (byteIndex == m_pcsptr.MEM.pc && m_pcsptr.MEM.initialized && !m_pcsptr.MEM.invalid) {
                emit textChanged(Stage::MEM, m_parserPtr->genStringRepr(memRead(row * 4)));
                if (byteIndex == maxInstr) {
                    emit textChanged(Stage::EX, "");
                }
                retStrings << "MEM";
            }
            if (byteIndex == m_pcsptr.WB.pc && m_pcsptr.WB.initialized && !m_pcsptr.WB.invalid) {
                emit textChanged(Stage::WB, m_parserPtr->genStringRepr(memRead(row * 4)));
                if (byteIndex == maxInstr) {
                    emit textChanged(Stage::MEM, "");
                }
                retStrings << "WB";
            }
            if (retStrings.isEmpty()) {
                // Clear invalid PC values for each stage (used when resetting the simlation)
                VALIDATE(IF);
                VALIDATE(ID);
                VALIDATE(EX);
                VALIDATE(MEM);
                VALIDATE(WB);
                return QVariant();
            } else {
                // Join strings by "/"
                return retStrings.join("/");
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
