#include "codeeditor.h"

#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QTextBlock>

#include <iterator>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
  m_lineNumberArea = new LineNumberArea(this);
  m_breakpointArea = new BreakpointArea(this);

  connect(this, SIGNAL(blockCountChanged(int)), this,
          SLOT(updateSidebarWidth(int)));

  connect(this, SIGNAL(updateRequest(QRect, int)), this,
          SLOT(updateSidebar(QRect, int)));

  connect(this, SIGNAL(cursorPositionChanged()), this,
          SLOT(highlightCurrentLine()));

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

  // Remove breakpoints if a breakpoint line has been removed
  auto a = blockCount();
  while (!m_breakpoints.empty() &&
         *(m_breakpoints.rbegin()) > (blockCount() - 1)) {
    m_breakpoints.erase(std::prev(m_breakpoints.end()));
  }
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
      if (m_breakpoints.find(blockNumber) != m_breakpoints.end()) {
        painter.drawPixmap(
            m_breakpointArea->padding, top, m_breakpointArea->imageWidth,
            m_breakpointArea->imageHeight, m_breakpointArea->m_breakpoint);
      }
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
}

void CodeEditor::breakpointClick(QMouseEvent *event, int forceState) {

  // Get line height
  QTextBlock block = firstVisibleBlock();
  auto height = blockBoundingRect(block).height();

  // Find block index in the codeeditor
  int index;
  if (block == document()->findBlockByLineNumber(0)) {
    index = -1 + (event->pos().y() + contentOffset().y()) / height;
  } else {
    index = (event->pos().y() + contentOffset().y()) / height;
  }
  // Get actual block index
  while (index > 0) {
    block = block.next();
    index--;
  }
  // Set or unset breakpoint
  int blockNumber = block.blockNumber();
  if (block.isValid()) {
    auto brkptIter = m_breakpoints.find(blockNumber);
    // Set/unset breakpoint
    if (forceState == 1) {
      m_breakpoints.insert(blockNumber);
    } else if (forceState == 2) {
      if (brkptIter != m_breakpoints.end())
        m_breakpoints.erase(m_breakpoints.find(blockNumber));
    } else {
      if (brkptIter != m_breakpoints.end()) {
        m_breakpoints.erase(brkptIter);
      } else {
        m_breakpoints.insert(blockNumber);
      }
    }
    repaint();
  }
}

BreakpointArea::BreakpointArea(CodeEditor *editor) : QWidget(editor) {
  codeEditor = editor;
  setCursor(Qt::PointingHandCursor);

  m_removeAction = new QAction("Remove breakpoint", this);
  m_removeAllAction = new QAction("Remove all breakpoints", this);
  m_addAction = new QAction("Add breakpoint", this);

  connect(m_removeAction, &QAction::triggered,
          [=] { codeEditor->breakpointClick(m_event, 2); });
  connect(m_addAction, &QAction::triggered,
          [=] { codeEditor->breakpointClick(m_event, 1); });
  connect(m_removeAllAction, &QAction::triggered, [=] {
    codeEditor->clearBreakpoints();
    repaint();
  });

  // Construct default mouseButtonEvent
  m_event = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(0, 0),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
}

void BreakpointArea::contextMenuEvent(QContextMenuEvent *event) {
  // setup context menu
  QMenu contextMenu;

  // Translate event to a QMouseEvent in case add/remove single breakpoint is
  // triggered
  *m_event = QMouseEvent(QEvent::MouseButtonRelease, event->pos(),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

  contextMenu.addAction(m_addAction);
  contextMenu.addAction(m_removeAction);
  contextMenu.addAction(m_removeAllAction);

  contextMenu.exec(event->globalPos());
}
