#include "csyntaxhighlighter.h"

#include "defines.h"

namespace Ripes {

CSyntaxHighlighter::CSyntaxHighlighter(QTextDocument* parent, std::shared_ptr<Assembler::Errors> errors)
    : SyntaxHighlighter(parent, errors) {
    HighlightingRule rule;

    errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    errorFormat.setUnderlineColor(Qt::red);

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bconst\\b"),    QStringLiteral("\\benum\\b"),     QStringLiteral("\\binline\\b"),
        QStringLiteral("\\bshort\\b"),    QStringLiteral("\\bstatic\\b"),   QStringLiteral("\\bstruct\\b"),
        QStringLiteral("\\btypedef\\b"),  QStringLiteral("\\btypename\\b"), QStringLiteral("\\bunion\\b"),
        QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bbreak\\b"),    QStringLiteral("\\bcase\\b"),
        QStringLiteral("\\bif\\b"),       QStringLiteral("\\belse\\b"),     QStringLiteral("\\bdo\\b"),
        QStringLiteral("\\bwhile\\b"),    QStringLiteral("\\bcontinue\\b"), QStringLiteral("\\bfor\\b"),
        QStringLiteral("\\bextern\\b"),   QStringLiteral("\\bgoto\\b"),     QStringLiteral("\\bswitch\\b"),
        QStringLiteral("\\bregister\\b"), QStringLiteral("\\breturn\\b"),   QStringLiteral("\\bsizeof\\b"),
        QStringLiteral("\\b__asm__\\b"),  QStringLiteral("\\basm\\b")};

    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    typeFormat.setForeground(Qt::darkBlue);
    typeFormat.setFontWeight(QFont::Bold);
    const QString typePatterns[] = {QStringLiteral("\\bchar\\b"),   QStringLiteral("\\bfloat\\b"),
                                    QStringLiteral("\\bdouble\\b"), QStringLiteral("\\bint\\b"),
                                    QStringLiteral("\\blong\\b"),   QStringLiteral("\\bshort\\b"),
                                    QStringLiteral("\\bsigned\\b"), QStringLiteral("\\bunsigned\\b"),
                                    QStringLiteral("\\bvoid\\b"),   QStringLiteral("\\bbool\\b")};
    for (const QString& pattern : typePatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = typeFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(QColor(Colors::Medalist));
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(QColor(Colors::Medalist));

    preprocessorFormat.setForeground(QColor(Qt::magenta).darker());
    rule.pattern = QRegularExpression(QStringLiteral("^\\ *#[^ ]*"));
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(QColor(0x800000));
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setForeground(QColor(Colors::BerkeleyBlue));
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
    commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}  // namespace Ripes

void CSyntaxHighlighter::syntaxHighlightBlock(const QString& text) {
    for (const HighlightingRule& rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

}  // namespace Ripes
