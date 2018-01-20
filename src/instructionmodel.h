#ifndef INSTRUCTIONMODEL_H
#define INSTRUCTIONMODEL_H

#include <QColor>
#include <set>
#include "defines.h"
#include "mainmemory.h"

#include <QAbstractTableModel>

class Parser;
class Pipeline;
class InstructionModel : public QAbstractTableModel {
    Q_OBJECT
public:
    InstructionModel(const StagePCS& pcsptr, const StagePCS& pcsptrPre, Parser* parser = nullptr,
                     QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void update();

    int m_currentIFrow = 0;

private:
    const StagePCS& m_pcsptr;
    const StagePCS& m_pcsptrPre;
    MainMemory* m_memory;
    Pipeline* m_pipelinePtr;
    const Parser* m_parserPtr;
    int m_textSize = 0;  // text segment, in bytes

signals:
    void textChanged(Stage stage, QString text, QColor col = QColor()) const;
    void currentIFRow(int) const;

public slots:
};

#endif  // INSTRUCTIONMODEL_H
