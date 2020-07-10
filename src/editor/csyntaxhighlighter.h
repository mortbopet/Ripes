#pragma once

#include <QRegularExpression>

#include "syntaxhighlighter.h"

/** Heavily based on QT's rich text syntax highlighter example.
 http://doc.qt.io/qt-5/qtwidgets-richtext-syntaxhighlighter-example.html
 */

namespace Ripes {

class CSyntaxHighlighter : public SyntaxHighlighter {
public:
    CSyntaxHighlighter(QTextDocument* parent = nullptr);

    void reset() override;
    bool acceptsSyntax() const override;
    void highlightBlock(const QString& text) override;

public slots:
    void clearAndRehighlight() override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat preprocessorFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

}  // namespace Ripes
