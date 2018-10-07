#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QByteArray>
#include <QTextDocument>

class Assembler {
public:
    Assembler();
    const QByteArray& assembleBinaryFile(const QTextDocument& doc);
    bool hasError() { return m_error; }
    bool hasData() { return m_hasData; }
    const QByteArray& getTextSegment() { return m_textSegment; }
    const QByteArray& getDataSegment() { return m_dataSegment; }
    void clear() { m_textSegment.clear(); }

private:
    uint32_t getRegisterNumber(const QString& reg);
    void unpackPseudoOp(const QStringList& fields, int& pos);
    void unpackOp(const QStringList& fields, int& pos);
    void assembleAssemblerDirective(const QStringList& fields);
    void assembleWords(const QStringList& fields, QByteArray& byteArr, size_t size);
    void assembleZeroArray(QByteArray& byteArray, size_t size);
    void restart();
    int getImmediate(QString string, bool& canConvert);
    QByteArray uintToByteArr(uint32_t);

    QMap<QString, int> m_labelPosMap;  // Map storing unpacked label

    QMap<int, QString>
        m_lineLabelUsageMap;  // Lines that need to be updated with label values (offsets) after unpacking is finished
    QMap<int, QStringList> m_instructionsMap;  // Map containing unpacked and offset-modified instruction

    QByteArray m_textSegment;
    QByteArray m_dataSegment;
    bool m_error = false;
    bool m_hasData = false;
    bool m_inDataSegment = false;  // Set when stating .data directive. Following instructions will be added to the data
                                   // segment of the program

    // Assembler functions
    void assembleInstruction(const QStringList& fields, int row);
    QByteArray assembleOpImmInstruction(const QStringList& fields, int row);
    QByteArray assembleOpInstruction(const QStringList& fields, int row);
    QByteArray assembleStoreInstruction(const QStringList& fields, int row);
    QByteArray assembleLoadInstruction(const QStringList& fields, int row);
    QByteArray assembleBranchInstruction(const QStringList& fields, int row);
    QByteArray assembleAuipcInstruction(const QStringList& fields, int row);
    QByteArray assembleJalrInstruction(const QStringList& fields, int row);
};

#endif  // ASSEMBLER_H
