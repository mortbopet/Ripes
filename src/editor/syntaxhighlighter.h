#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>

namespace Ripes {

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument* parent = nullptr);

    QString getTooltipForLine(int index) const;

    virtual void highlightBlock(const QString& text) = 0;
    virtual void reset();
    virtual QString checkSyntax(const QString& line) = 0;

    virtual bool acceptsSyntax() const = 0;

signals:
    void rehighlightInvalidBlock(const QTextBlock&);

public slots:
    virtual void clearAndRehighlight() = 0;

protected:
    void setTooltip(int line, QString tooltip);
    void clearTooltip(int line);
    /**
     * @brief m_tooltipForLine
     * The syntax highlighter may provide a tooltip for each line in the current text document. These tooltips will be
     * displayed by the codeeditor when a relevant tooltip event occurs.
     */
    std::map<int, QString> m_tooltipForLine;
};
}  // namespace Ripes
