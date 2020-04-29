#include "syntaxhighlighter.h"

#include <QTextDocument>

namespace Ripes {
SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    connect(this->document(), &QTextDocument::blockCountChanged, this, &SyntaxHighlighter::handleBlockCountChanged);
}

void SyntaxHighlighter::handleBlockCountChanged() {
    clearAndRehighlight();
    const auto tooltipsCopy = m_tooltipForLine;  // deep copy
    // Remove any  tooltip for lines which have been deleted
    for (const auto& tooltip : tooltipsCopy) {
        if (tooltip.first >= this->document()->blockCount()) {
            m_tooltipForLine.erase(tooltip.first);
        }
    }
}

QString SyntaxHighlighter::getTooltipForLine(int index) const {
    if (m_tooltipForLine.count(index)) {
        return m_tooltipForLine.at(index);
    } else {
        return QString();
    }
}

void SyntaxHighlighter::setTooltip(int line, QString tooltip) {
    m_tooltipForLine[line] = tooltip;
}
void SyntaxHighlighter::clearTooltip(int line) {
    m_tooltipForLine.erase(line);
}

void SyntaxHighlighter::reset() {
    m_tooltipForLine.clear();
}

}  // namespace Ripes
