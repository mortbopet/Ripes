#ifndef INSTRUCTIONMODEL_H
#define INSTRUCTIONMODEL_H

#include "defines.h"
#include "mainmemory.h"

#include <QAbstractTableModel>

class Parser;
class InstructionModel : public QAbstractTableModel {
    Q_OBJECT
public:
    InstructionModel(const StagePCS& pcsptr, Parser* parser = nullptr, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void update();

private:
    const StagePCS& m_pcsptr;
    MainMemory* m_memory;
    Parser* m_parserPtr;
    int m_textSize = 0;  // text segment, in bytes

    uint32_t memRead(uint32_t address) const;

signals:

public slots:
};

#endif  // INSTRUCTIONMODEL_H
