#include "rvsyntaxhighlighter.h"

#include "colors.h"

namespace Ripes {

RVSyntaxHighlighter::RVSyntaxHighlighter(
    QTextDocument *parent, std::shared_ptr<Errors> errors,
    const std::set<QString> &supportedOpcodes)
    : SyntaxHighlighter(parent, errors) {
  HighlightingRule rule;

  // General registers
  registerFormat.setForeground(QColor{0x80, 0x00, 0x00});
  rule.pattern = QRegularExpression("\\b[(a|s|t|x)][0-9]{1,2}");
  rule.format = registerFormat;
  m_highlightingRules.append(rule);

  // Name-specific registers
  QStringList registerPatterns;
  registerPatterns << "\\bzero\\b"
                   << "\\bra\\b"
                   << "\\bsp\\b"
                   << "\\bgp\\b"
                   << "\\btp\\b"
                   << "\\bfp\\b";
  for (const auto &pattern : std::as_const(registerPatterns)) {
    rule.pattern = QRegularExpression(pattern);
    rule.format = registerFormat;
    m_highlightingRules.append(rule);
  }

  // Instructions
  instructionFormat.setForeground(Colors::BerkeleyBlue);
  for (const auto &pattern : supportedOpcodes) {
    const QString regexPattern = "\\b" + pattern + "\\b";
    rule.pattern = QRegularExpression(regexPattern);
    rule.format = instructionFormat;
    m_highlightingRules.append(rule);
  }

  // Labels
  labelFormat.setForeground(Colors::Medalist);
  rule.pattern = QRegularExpression(R"([\S]+:)");
  rule.format = labelFormat;
  m_highlightingRules.append(rule);

  // Immediates
  immediateFormat.setForeground(QColorConstants::DarkGreen);
  rule.pattern = QRegularExpression("\\b(?<![A-Za-z])[-+]?\\d+");
  rule.format = immediateFormat;
  m_highlightingRules.append(rule);

  // Prefixed immediates (0x, 0b)
  rule.pattern =
      QRegularExpression("([-+]?0[xX][0-9a-fA-F]+|[-+]?0[bB][0-1]+)");
  m_highlightingRules.append(rule);

  // Strings
  stringFormat.setForeground(QColor{0x80, 0x00, 0x00});
  rule.pattern = QRegularExpression(R"("(?:[^"]|\.)*")");
  rule.format = stringFormat;
  m_highlightingRules.append(rule);

  // Comments
  commentFormat.setForeground(Colors::Medalist);
  rule.pattern = QRegularExpression("[#]+.*");
  rule.format = commentFormat;
  m_highlightingRules.append(rule);
}

void RVSyntaxHighlighter::syntaxHighlightBlock(const QString &text) {
  for (const HighlightingRule &rule : std::as_const(m_highlightingRules)) {
    QRegularExpressionMatchIterator matchIterator =
        rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}

} // namespace Ripes
