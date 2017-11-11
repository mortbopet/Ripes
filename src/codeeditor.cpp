#include "codeeditor.h"

#include <QPainter>
#include <QTextBlock>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
  m_lineNumberArea = new LineNumberArea(this);
  m_breakpointArea = new BreakpointArea(this);

  connect(this, SIGNAL(blockCountChanged(int)), this,
          SLOT(updateSidebarWidth(int)));

  connect(this, SIGNAL(updateRequest(QRect, int)), this,
          SLOT(updateSidebar(QRect, int)));

  connect(this, SIGNAL(cursorPositionChanged()), this,
          SLOT(highlightCurrentLine()));

  // add some breakpoints
  m_breakpoints = QList<int>() << 3 << 5 << 10 << 20;

  updateSidebarWidth(0);
  highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }
  int space = 3 + fontMetrics().width(QLatin1Char('15')) * digits;
  return space;
}

void CodeEditor::updateSidebarWidth(int /* newBlockCount */) {
  // Set margins of the text edit area
  setViewportMargins(lineNumberAreaWidth() + m_breakpointArea->width(), 0, 0,
                     0);
}

void CodeEditor::updateSidebar(const QRect &rect, int dy) {
  if (dy) {
    m_lineNumberArea->scroll(0, dy);
    m_breakpointArea->scroll(0, dy);
  } else {
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(),
                             rect.height());
    m_breakpointArea->update(0, rect.y(), m_breakpointArea->width(),
                             rect.height());
  }

  if (rect.contains(viewport()->rect()))
    updateSidebarWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(QRect(cr.left() + m_breakpointArea->width(),
                                      cr.top(), lineNumberAreaWidth(),
                                      cr.height()));

  m_breakpointArea->setGeometry(cr.left(), cr.top(), m_breakpointArea->width(),
                                cr.height());
}

void CodeEditor::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(Qt::yellow).lighter(160);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(m_lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.drawText(0, top, m_lineNumberArea->width(),
                       fontMetrics().height(), Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
}

void CodeEditor::breakpointAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(m_breakpointArea);
  painter.fillRect(event->rect(), Qt::lightGray);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      if (m_breakpoints.contains(blockNumber)) {
        painter.drawPixmap(m_breakpointArea->padding, top,
                           m_breakpointArea->imageWidth,
                           m_breakpointArea->imageHeight,
                           m_breakpointArea->m_breakpoint.scaledToHeight(
                               fontMetrics().height() * 0.7));
      }
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
}

void BreakpointArea::mouseReleaseEvent(QMouseEvent *event) { return; }
