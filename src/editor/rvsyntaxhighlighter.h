#pragma once

#include <QRegularExpression>

#include "syntaxhighlighter.h"

namespace Ripes {

class RVSyntaxHighlighter : public SyntaxHighlighter {
public:
    RVSyntaxHighlighter(QTextDocument* parent = nullptr, std::shared_ptr<AssemblerTmp::Errors> errors = {});
    void syntaxHighlightBlock(const QString& text) override;

private:
};

}  // namespace Ripes
