#include "highlightabletextedit.h"

#include "STLExtras.h"

#include <QPainter>
#include <QTextBlock>

#include "colors.h"

namespace Ripes {

HighlightableTextEdit::HighlightableTextEdit(QWidget *parent)
    : QPlainTextEdit(parent) {
  // Clear block highlighting on text inserted or removed. This avoids clearing
  // block highlightins on formatting changes.
  connect(this->document(), &QTextDocument::contentsChange, this,
          [&](int /*pos*/, int charsRemoved, int charsAdded) {
            if ((charsRemoved == 0 && charsAdded == 0) ||
                (charsAdded == charsRemoved))
              return;
            clearBlockHighlights();
          });
}

void HighlightableTextEdit::paintEvent(QPaintEvent *event) {
  QPlainTextEdit::paintEvent(event);

  // Draw stage names for highlighted addresses
  QPainter painter(viewport());

  for (const auto &hb : m_highlightedBlocksText) {
    if (!hb.first.isValid())
      continue;
    const QString stageString = hb.second.join('/');
    const auto bbr = blockBoundingGeometry(hb.first);
    painter.setFont(font());
    const auto fm = painter.fontMetrics();
    const auto advance = fm.horizontalAdvance(stageString);
    const auto height = fm.height();
    QPointF drawAt =
        QPointF(bbr.width() - advance - /*right-hand side padding*/ 10,
                bbr.top() + (bbr.height() / 2.0 - fm.height() / 2.0));
    painter.drawText(QRectF(drawAt.x(), drawAt.y(), advance, fm.height()),
                     stageString);
  }
  painter.end();
}

void HighlightableTextEdit::applyHighlighting() {
  QList<QTextEdit::ExtraSelection> selections;
  for (auto &bh : m_blockHighlights) {
    if (auto selection = getExtraSelection(bh); selection.has_value())
      selections.push_back(selection.value());
  }
  setExtraSelections(selections);

  // The block text is drawn on the viewport itself, which is not automatically
  // redrawn when the extra selections change. So, to ensure that the new stage
  // text is written, also update the viewport.
  viewport()->update();
}

void HighlightableTextEdit::clearBlockHighlights() {
  // A memory occurs within QPlainTextEdit::clear if extra selections has been
  // set. This is most possibly a bug, but seems to be fixed if we manually
  // clear the selections before we clear (and add new text) to the text edit.
  setExtraSelections({});

  m_highlightedBlocksText.clear();
  m_highlightedBlocks.clear();
  m_blockHighlights.clear();
  viewport()->update();
  update();
}

void HighlightableTextEdit::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);
  applyHighlighting();
}

void HighlightableTextEdit::highlightBlock(const QTextBlock &block,
                                           const QColor &color,
                                           const QString &text) {
  if (!block.isValid())
    return;
  if (!text.isEmpty())
    m_highlightedBlocksText[block].push_back(text);

  // Check if we're already highlighting the block. If this is the case, do not
  // set an additional highlight on it.
  if (m_highlightedBlocks.count(block))
    return;
  m_blockHighlights.push_back({});
  auto &highlight = m_blockHighlights.back();
  highlight.blockNumber = block.blockNumber();
  highlight.color = color;
  applyHighlighting();
}

std::optional<QTextEdit::ExtraSelection>
HighlightableTextEdit::getExtraSelection(
    const HighlightableTextEdit::BlockHighlight &highlighting) {
  auto block = document()->findBlockByNumber(highlighting.blockNumber);
  if (!block.isValid())
    return {};

  QTextEdit::ExtraSelection selection;
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = QTextCursor(block);
  const auto bbr = blockBoundingRect(block);
  QLinearGradient grad(bbr.topLeft(), bbr.bottomRight());
  grad.setColorAt(0, palette().base().color());
  grad.setColorAt(1, highlighting.color);
  selection.format.setBackground(grad);
  return {selection};
}

} // namespace Ripes
