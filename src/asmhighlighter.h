#ifndef ASMHIGHLIGHTER_H
#define ASMHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>

/* Class for highlighting RISC-V assembly code
 Based on QT's rich text syntax highlighter example.
 http://doc.qt.io/qt-5/qtwidgets-richtext-syntaxhighlighter-example.html

 Matches instruction names directly, and register aliases/true name.
 Matches immediate values by regex*/
namespace {
enum class Type { Immediate, Register, Offset };
}

class FieldType {
  // Class for defining field-specific rules, such as immediate range checking,
  // and whether a register is recognized
public:
  explicit FieldType(Type type, int lowerBound = 0, int upperBound = 0);
  QString validateField(const QString &field) const;
  Type m_type;
  int m_lowerBound;
  int m_upperBound;
};

class AsmHighlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  explicit AsmHighlighter(QTextDocument *parent = nullptr);

  void highlightBlock(const QString &text);

  QString checkSyntax(const QString &line);

signals:

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };

  struct SyntaxRule {
    QString instr;
    int fields; // n instruction fields, including the instruction
    QList<FieldType>
        inputs; // list of each accepted input for the instruction, in order
  };

  void createSyntaxRules();

  QMap<QString, SyntaxRule> m_syntaxRules;

  QVector<HighlightingRule> m_highlightingRules;

  // Format type for each matching case
  QTextCharFormat regFormat;
  QTextCharFormat instrFormat;
  QTextCharFormat immFormat;
  QTextCharFormat commentFormat;
  QTextCharFormat errorFormat;

  QMap<int, QString> m_errors;

public slots:
};

#endif // ASMHIGHLIGHTER_H
