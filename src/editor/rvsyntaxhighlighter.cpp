#include "rvsyntaxhighlighter.h"

#include "defines.h"

namespace Ripes {

RVSyntaxHighlighter::RVSyntaxHighlighter(QTextDocument* parent, std::shared_ptr<AssemblerTmp::Errors> errors)
    : SyntaxHighlighter(parent, errors) {}  // namespace Ripes

void RVSyntaxHighlighter::syntaxHighlightBlock(const QString& text) {}

}  // namespace Ripes
