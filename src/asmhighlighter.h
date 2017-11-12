#ifndef ASMHIGHLIGHTER_H
#define ASMHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>

/* Class for highlighting RISC-V assembly code
 Based on QT's rich text syntax highlighter example.
 http://doc.qt.io/qt-5/qtwidgets-richtext-syntaxhighlighter-example.html

 Matches instruction names directly, and register aliases/true name.
 Matches immediate values by regex*/

class AsmHighlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  explicit AsmHighlighter(QTextDocument *parent = nullptr);

  void highlightBlock(const QString &text);

signals:

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };

  QVector<HighlightingRule> m_highlightingRules;

  // Format type for each matching case
  QTextCharFormat regFormat;
  QTextCharFormat instrFormat;
  QTextCharFormat immFormat;
  QTextCharFormat commentFormat;

public slots:
};

#endif // ASMHIGHLIGHTER_H
