#include "rvsyntaxhighlighter.h"

#include "colors.h"

namespace Ripes {

RVSyntaxHighlighter::RVSyntaxHighlighter(
    QTextDocument *parent, std::shared_ptr<Errors> errors,
    const std::set<QString> &supportedOpcodes)
    : SyntaxHighlighter(parent, errors) {
  HighlightingRule rule;

  const SyntaxColorScheme colors = currentSyntaxColors();

  // General registers
  registerFormat.setForeground(colors.registerName);
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
  instructionFormat.setForeground(colors.instruction);
  for (const auto &pattern : supportedOpcodes) {
    const QString regexPattern = "\\b" + pattern + "\\b";
    rule.pattern = QRegularExpression(regexPattern);
    rule.format = instructionFormat;
    m_highlightingRules.append(rule);
  }

  // Labels
  labelFormat.setForeground(colors.label);
  rule.pattern = QRegularExpression(R"([\S]+:)");
  rule.format = labelFormat;
  m_highlightingRules.append(rule);

  // Immediates
  immediateFormat.setForeground(colors.immediate);
  rule.pattern = QRegularExpression("\\b(?<![A-Za-z])[-+]?\\d+");
  rule.format = immediateFormat;
  m_highlightingRules.append(rule);

  // Prefixed immediates (0x, 0b)
  rule.pattern =
      QRegularExpression("([-+]?0[xX][0-9a-fA-F]+|[-+]?0[bB][0-1]+)");
  m_highlightingRules.append(rule);

  // Strings
  stringFormat.setForeground(colors.string);
  rule.pattern = QRegularExpression(R"("(?:[^"]|\.)*")");
  rule.format = stringFormat;
  m_highlightingRules.append(rule);

  // Comments
  commentFormat.setForeground(colors.comment);
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
