#include "asmhighlighter.h"

#include <QRegularExpressionMatchIterator>

#include "defines.h"

AsmHighlighter::AsmHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {

  HighlightingRule rule;

  // Create rules for name-specific registers
  regFormat.setForeground(QColor(Colors::FoundersRock));
  QStringList registerPatterns;
  registerPatterns << "\\bzero\\b"
                   << "\\bra\\b"
                   << "\\bsp\\b"
                   << "\\bgp\\b"
                   << "\\btp\\b"
                   << "\\bfp\\b";
  for (const auto &pattern : registerPatterns) {
    rule.pattern = QRegularExpression(pattern);
    rule.format = regFormat;
    m_highlightingRules.append(rule);
  }

  // Create match cases for instructions
  QStringList instructionPatterns;
  instructionPatterns << "\\bla\\b"
                      << "\\brd\\b"
                      << "\\blw\\b"
                      << "\\blh\\b"
                      << "\\blb\\b"
                      << "\\bsb\\b"
                      << "\\bsh\\b"
                      << "\\bsw\\b"
                      << "\\bnop\\b"
                      << "\\bli\\b"
                      << "\\bmv\\b"
                      << "\\bnot\\b"
                      << "\\bneg\\b"
                      << "\\bnegw\\b"
                      << "\\bsext.w\\b"
                      << "\\bseqz\\b"
                      << "\\bsnez\\b"
                      << "\\bsltz\\b"
                      << "\\bsgtz\\b"
                      << "\\bbegz\\b"
                      << "\\bbnez\\b"
                      << "\\bblez\\b"
                      << "\\bbgez\\b"
                      << "\\bbltz\\b"
                      << "\\bbgtz\\b"
                      << "\\bbgt\\b"
                      << "\\bble\\b"
                      << "\\bbgtu\\b"
                      << "\\bbleu\\b"
                      << "\\bj\\b"
                      << "\\bjal\\b"
                      << "\\bjr\\b"
                      << "\\bjalr\\b"
                      << "\\bret\\b"
                      << "\\bcall\\b"
                      << "\\btail\\b"
                      << "\\bfence\\b"
                      << "\\brdinstret\\b"
                      << "\\brdcycle\\b"
                      << "\\brdtime\\b"
                      << "\\bcsrr\\b"
                      << "\\bcsrw\\b"
                      << "\\bcsrs\\b"
                      << "\\bcsrc\\b"
                      << "\\bcsrwi\\b"
                      << "\\bcsrsi\\b"
                      << "\\bcsrci\\b"
                      << "\\bauipc\\b"
                      << "\\baddi\\b"
                      << "\\baddi\\b"
                      << "\\bxori\\b"
                      << "\\bsub\\b"
                      << "\\bsubw\\b"
                      << "\\baddiw\\b"
                      << "\\bsltiu\\b"
                      << "\\bsltu\\b"
                      << "\\bslt\\b"
                      << "\\bbeq\\b"
                      << "\\bbne\\b"
                      << "\\bbge\\b"
                      << "\\bblt\\b"
                      << "\\bbltu\\b"
                      << "\\bbgeu\\b"
                      << "\\bsrli\\b"
                      << "\\bslli\\b";
  instrFormat.setForeground(QColor(Colors::BerkeleyBlue));
  for (const auto &pattern : instructionPatterns) {
    rule.pattern = QRegularExpression(pattern);
    rule.format = instrFormat;
    m_highlightingRules.append(rule);
  }

  // Create rules for registers that can easily be regex matched
  // (saved, temporary and argument registers)
  rule.pattern = QRegularExpression("\\b[(a|s|t)][0-9]{1,2}");
  rule.format = regFormat;
  m_highlightingRules.append(rule);

  // Create immediate hightlighting rule
  immFormat.setForeground(QColor(Qt::darkGreen));
  rule.pattern = QRegularExpression("(?<![A-Za-z])[-+]?\\d+");
  rule.format = immFormat;
  m_highlightingRules.append(rule);

  // Create comment highlighting rule
  commentFormat.setForeground(QColor(Colors::Medalist));
  rule.pattern = QRegularExpression("[#]+.*");
  rule.format = commentFormat;
  m_highlightingRules.append(rule);
}

void AsmHighlighter::highlightBlock(const QString &text) {
  foreach (const HighlightingRule &rule, m_highlightingRules) {
    QRegularExpressionMatchIterator matchIterator =
        rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}
