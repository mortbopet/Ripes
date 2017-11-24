#ifndef INSTRUCTIONMODEL_H
#define INSTRUCTIONMODEL_H

#include <QStandardItemModel>

class InstructionModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit InstructionModel(QObject *parent = nullptr);

signals:

public slots:
};

#endif // INSTRUCTIONMODEL_H
