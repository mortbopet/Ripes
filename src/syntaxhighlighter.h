#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>

/* Class for highlighting RISC-V assembly code Based on QT's rich text syntax highlighter example.
 http://doc.qt.io/qt-5/qtwidgets-richtext-syntaxhighlighter-example.html

 Matches instruction names directly, and register aliases/true name.
 Matches immediate values by regex*/
enum class Type { Immediate, Register, Offset, String };

class SyntaxHighlighter;
class FieldType {
    // Class for defining field-specific rules, such as immediate range checking,
    // and whether a register is recognized
public:
    explicit FieldType(Type type, int lowerBound = 0, int upperBound = 0, SyntaxHighlighter* highlighter = nullptr);
    QString validateField(const QString& field) const;
    Type m_type;
    int m_lowerBound;
    int m_upperBound;

    SyntaxHighlighter* m_highlighter;
};

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
    friend class FieldType;

public:
    explicit SyntaxHighlighter(QTextDocument* parent = nullptr);

    void highlightBlock(const QString& text) override;
    void reset();

    QString checkSyntax(const QString& line);

signals:
    void setTooltip(int, QString);
    void rehighlightInvalidBlock(const QTextBlock&);

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    struct SyntaxRule {
        QString instr;
        int fields;                   // n instruction fields, including the instruction
        int fields2;                  // jal and jalr can be 2 different ops (pseudo- and base op)
        bool skipFirstField = false;  // jal needs to skip its first operand for its pseudo-op
        bool hasListField = false;    // .byte/.word etc. can accept a list of arguments. Last argument in the inputs
                                      // fieldtype list can be a list
        QList<FieldType> inputs;      // list of each accepted input for the instruction, in order
    };

    void createSyntaxRules();

    QMap<QString, QList<SyntaxRule>> m_syntaxRules;  // Maps instruction names to syntax rule(s)

    QVector<HighlightingRule> m_highlightingRules;

    // Format type for each matching case
    QTextCharFormat regFormat;
    QTextCharFormat instrFormat;
    QTextCharFormat immFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat errorFormat;

    QMap<QString, int> m_labelPosMap;
    QMap<int, QString> m_posLabelMap;
    QSet<int> m_rowsUsingLabels;  // List of lines that are rehighlighted once the main syntax checking is done -
                                  // needed for supporting labels that are declared after usage

public slots:
    void invalidateLabels(const QTextCursor&);
    void clearAndRehighlight();
};

#endif  // SYNTAXHIGHLIGHTER_H
