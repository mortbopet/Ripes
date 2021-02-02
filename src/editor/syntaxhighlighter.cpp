#include "syntaxhighlighter.h"

#include <QTextDocument>

namespace Ripes {
SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent, std::shared_ptr<Assembler::Errors> errors)
    : QSyntaxHighlighter(parent), m_errors(errors) {
    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorFormat.setUnderlineColor(Qt::red);
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
    int row = currentBlock().firstLineNumber();
    if (m_errors && m_errors->toMap().count(row) != 0) {
        setFormat(0, text.length(), errorFormat);
    } else {
        syntaxHighlightBlock(text);
    }
}

}  // namespace Ripes
