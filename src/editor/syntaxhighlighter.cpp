#include "syntaxhighlighter.h"

#include <QTextDocument>
#include <utility>

namespace Ripes {
SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent,
                                     std::shared_ptr<Errors> errors)
    : QSyntaxHighlighter(parent), m_errors(std::move(errors)) {
  errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  errorFormat.setUnderlineColor(Qt::red);
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
  int row = currentBlock().firstLineNumber();
  if (m_errors && m_errors->toMap().contains(row)) {
    setFormat(0, text.length(), errorFormat);
  } else {
    syntaxHighlightBlock(text);
  }
}

} // namespace Ripes
