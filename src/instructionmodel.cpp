#include "instructionmodel.h"
#include "parser.h"
#include "pipeline.h"

#include <QHeaderView>

InstructionModel::InstructionModel(const StagePCS& pcsptr, const StagePCS& pcsptrPre, Parser* parser, QObject* parent)
    : m_pcsptr(pcsptr), m_pcsptrPre(pcsptrPre), m_parserPtr(parser), QAbstractTableModel(parent) {
    m_pipelinePtr = Pipeline::getPipeline();
    m_memory = m_pipelinePtr->getRuntimeMemoryPtr();
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
    return 4;
}

namespace {
#define VALIDATE(stage)                                                 \
    if (!m_pcsptr.stage.initialized) {                                  \
        emit textChanged(Stage::stage, "");                             \
    } else if (m_pcsptr.stage.invalidReason == 1) {                     \
        emit textChanged(Stage::stage, "nop (flush)", QColor(Qt::red)); \
    } else if (m_pcsptr.stage.invalidReason == 2) {                     \
        emit textChanged(Stage::stage, "nop (stall)", QColor(Qt::red)); \
    } else if (m_pcsptr.stage.invalidReason == 3) {                     \
        emit textChanged(Stage::stage, ""); /*EOF*/                     \
    }
}

QVariant InstructionModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    int row = index.row();
    switch (index.column()) {
        case 0: {
            // Breakpoints. Return true if breakpoint is found
            if (role == Qt::CheckStateRole) {
                if (m_pipelinePtr->m_breakpoints.find(row * 4) != m_pipelinePtr->m_breakpoints.end()) {
                    return true;
                } else {
                    return false;
                }
            }
            break;
        }
        case 1: {
            if (role == Qt::DisplayRole)
                return row * 4;
            break;
        }
        case 2: {
            if (role == Qt::DisplayRole) {
                // check if instruction is in any pipeline stage, and whether the given PC for the pipeline
                // stage is valid. Furthermore, because of branching, an instruction can be in multiple stages
                // at once - therefore, we build up a return string by checking all stage program counters
                QStringList retStrings;
                uint32_t byteIndex = row * 4;
                uint32_t maxInstr = m_textSize - 4;
                if (byteIndex == m_pcsptr.EX.pc && m_pcsptr.EX.initialized && m_pcsptr.EX.isValid()) {
                    emit textChanged(Stage::EX, m_parserPtr->getInstructionString(row * 4));
                    if (byteIndex == maxInstr) {
                        emit textChanged(Stage::ID, "");
                    }
                    retStrings << "EX";
                }
                if (byteIndex == m_pcsptr.ID.pc && m_pcsptr.ID.initialized && m_pcsptr.ID.isValid()) {
                    emit textChanged(Stage::ID, m_parserPtr->getInstructionString(row * 4));
                    if (byteIndex == maxInstr) {
                        emit textChanged(Stage::IF, "");
                    }
                    retStrings << "ID";
                }
                if (byteIndex == m_pcsptr.IF.pc && m_pcsptr.IF.initialized && m_pcsptr.IF.isValid()) {
                    emit textChanged(Stage::IF, m_parserPtr->getInstructionString(row * 4));
                    emit currentIFRow(row);  // for moving view to IF position
                    retStrings << "IF";
                }
                if (byteIndex == m_pcsptr.MEM.pc && m_pcsptr.MEM.initialized && m_pcsptr.MEM.isValid()) {
                    emit textChanged(Stage::MEM, m_parserPtr->getInstructionString(row * 4));
                    if (byteIndex == maxInstr) {
                        emit textChanged(Stage::EX, "");
                    }
                    retStrings << "MEM";
                }
                if (byteIndex == m_pcsptr.WB.pc && m_pcsptr.WB.initialized && m_pcsptr.WB.isValid()) {
                    emit textChanged(Stage::WB, m_parserPtr->getInstructionString(row * 4));
                    if (byteIndex == maxInstr) {
                        emit textChanged(Stage::MEM, "");
                    }
                    retStrings << "WB";
                }
                if (retStrings.isEmpty()) {
                    // Clear invalid PC values for each stage (used when resetting the program)
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
            break;
        }
        case 3:
            if (role == Qt::DisplayRole) {
                return m_parserPtr->getInstructionString(row * 4);
            }
        default:
            return QVariant();
    }
    return QVariant();
}

bool InstructionModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.column() == 0 && role == Qt::CheckStateRole) {
        if ((Qt::CheckState)value.toInt() == Qt::Checked) {
            if (m_pipelinePtr->m_breakpoints.find(index.row() * 4) != m_pipelinePtr->m_breakpoints.end()) {
                // Breakpoint is already set - remove breakpoint
                m_pipelinePtr->m_breakpoints.erase(index.row() * 4);
            } else {
                // Set breakpoint
                m_pipelinePtr->m_breakpoints.insert(index.row() * 4);
            }
        }
        return true;
    }
    return false;
}

Qt::ItemFlags InstructionModel::flags(const QModelIndex& index) const {
    auto def = Qt::ItemIsEnabled;
    if (index.column() == 0)
        return Qt::ItemIsUserCheckable | def;
    return def;
}

QVariant InstructionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return "BP";
            case 1:
                return "PC";
            case 2:
                return "Stage";
            case 3:
                return "Instruction";
            default:
                return QVariant();
        }
    }

    return QVariant();
}
