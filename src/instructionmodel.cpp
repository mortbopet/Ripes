#include "instructionmodel.h"

#include <QHeaderView>

InstructionModel::InstructionModel(QObject *parent) : QStandardItemModel(parent)
{

    setItem(0,2, new QStandardItem());
    setHeaderData(0, Qt::Horizontal, "PC");
    setHeaderData(1, Qt::Horizontal, "Stage");
    setHeaderData(2, Qt::Horizontal, "Instruction");
}
