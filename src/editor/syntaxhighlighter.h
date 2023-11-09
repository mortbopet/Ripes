#pragma once

#include <QSyntaxHighlighter>
#include <memory>

#include "isa/isa_defines.h"

namespace Ripes {

class SyntaxHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  SyntaxHighlighter(QTextDocument *parent = nullptr,
                    std::shared_ptr<Errors> errors = {});

  /**
   * @brief highlightBlock
   * Performs language-independent highlighting, such as line-underlining upon
   * an error during assembly/compilation.
   */
  void highlightBlock(const QString &text) override final;
  /**
   * @brief syntaxHighlightBlock
   * Performs language-specific highlighting
   */
  virtual void syntaxHighlightBlock(const QString &text) = 0;

protected:
  /**
   * @brief m_errors
   * The syntax highlighter may provide a tooltip error for each line in the
   * current text document. These tooltips will be displayed by the codeeditor
   * when a relevant tooltip event occurs.
   */
  std::shared_ptr<Errors> m_errors;
  QTextCharFormat errorFormat;
};
} // namespace Ripes
