#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QByteArray>
#include <QTextDocument>

class Assembler {
public:
    Assembler();
    const QByteArray& assembleBinaryFile(const QTextDocument& doc);
    bool hasError() { return m_error; }

private:
    uint32_t getRegisterNumber(const QString& reg);
    void unpackPseudoOp(const QStringList& fields, int& pos);
    void unpackOp(const QStringList& fields, int& pos);
    void restart();
    QByteArray uintToByteArr(uint32_t);

    QMap<QString, int> m_labelPosMap;  // Map storing unpacked label

    QMap<int, QString>
        m_lineLabelUsageMap;  // Lines that need to be updated with label values (offsets) after unpacking is finished
    QMap<int, QStringList> m_instructionsMap;  // Map containing unpacked and offset-modified instruction

    QByteArray m_outputArray;
    bool m_error = false;

    // Assembler functions
    QByteArray assembleInstruction(const QStringList& fields, int row);
    QByteArray assembleOpImmInstruction(const QStringList& fields, int row);
    QByteArray assembleOpInstruction(const QStringList& fields, int row);
    QByteArray assembleStoreInstruction(const QStringList& fields, int row);
    QByteArray assembleLoadInstruction(const QStringList& fields, int row);
    QByteArray assembleBranchInstruction(const QStringList& fields, int row);
    QByteArray assembleAuipcInstruction(const QStringList& fields, int row);
    QByteArray assembleJalrInstruction(const QStringList& fields, int row);
};

#endif  // ASSEMBLER_H
