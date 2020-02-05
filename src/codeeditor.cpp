#include "codeeditor.h"
#include "defines.h"

#include <QAction>
#include <QApplication>
#include <QLinearGradient>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QToolTip>
#include <QWheelEvent>

#include <iterator>

#include "processorhandler.h"

namespace Ripes {

CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent) {
    m_lineNumberArea = new LineNumberArea(this);
    m_breakpointArea = new BreakpointArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateSidebarWidth(int)));

    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateSidebar(QRect, int)));

    updateSidebarWidth(0);

    // Set font for the entire widget. calls to fontMetrics() will get the
    // dimensions of the currently set font
    m_font = QFont("Monospace");
    m_font.setStyleHint(QFont::TypeWriter);
    m_font.setPointSize(10);
    setFont(m_font);
    m_fontTimer.setSingleShot(true);

    // set event filter for catching scroll events
    installEventFilter(this);

    // needed for instant tooltip displaying
    setMouseTracking(true);

    setWordWrapMode(QTextOption::NoWrap);
    setupChangedTimer();
}

void CodeEditor::setupChangedTimer() {
    // configures the change-timer and assembler connectivity with Parser
    m_changeTimer.setInterval(500);
    m_changeTimer.setSingleShot(true);
    // A change in the document will start the timer - when the timer elapses, the contents will be assembled if there
    // is no syntax error. By doing this, the timer is restartet each time a change occurs (ie. a user is continuously
    // typing)
    connect(this, &QPlainTextEdit::textChanged, [=] { m_changeTimer.start(); });
    connect(&m_changeTimer, &QTimer::timeout, this, &CodeEditor::textChanged);
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int rightPadding = 6;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = rightPadding + fontMetrics().width(QString("1")) * digits;
    return space;
}

void CodeEditor::updateSidebarWidth(int /* newBlockCount */) {
    // Set margins of the text edit area
    m_sidebarWidth = lineNumberAreaWidth() + m_breakpointArea->width();
    setViewportMargins(m_sidebarWidth, 0, 0, 0);
}

bool CodeEditor::eventFilter(QObject* /*observed*/, QEvent* event) {
    // Event filter for catching ctrl+Scroll events, for text resizing
    if (event->type() == QEvent::Wheel && QApplication::keyboardModifiers() == Qt::ControlModifier) {
        auto wheelEvent = static_cast<QWheelEvent*>(event);
        // Since multiple wheelevents are issued on a scroll,
        // start a timer to only catch the first one

        // change font size
        if (!m_fontTimer.isActive()) {
            if (wheelEvent->angleDelta().y() > 0) {
                if (m_font.pointSize() < 30)
                    m_font.setPointSize(m_font.pointSize() + 1);
            } else {
                if (m_font.pointSize() > 6)
                    m_font.setPointSize(m_font.pointSize() - 1);
            }
            m_fontTimer.start(50);
        }
        setFont(m_font);
        return true;
    }

    return false;
}

void CodeEditor::updateTooltip(int line, QString tip) {
    // Connects to AsmHighlighter::setTooltip
    if (tip == QString()) {
        // unset tooltip - accepted syntax at line
        m_tooltipForLine.remove(line);
    } else {
        m_tooltipForLine[line] = tip;
    }
}

void CodeEditor::clearBreakpoints() {
    ProcessorHandler::get()->clearBreakpoints();
}

bool CodeEditor::event(QEvent* event) {
    // Override event handler for receiving tool tips
    if (event->type() == QEvent::ToolTip) {
        // Tooltips are updated through slot handler updateTooltip
        auto* helpEvent = static_cast<QHelpEvent*>(event);
        QTextCursor textAtCursor = cursorForPosition(helpEvent->pos());
        int row = textAtCursor.block().firstLineNumber();
        if (m_tooltipForLine.contains(row) && m_tooltipForLine[row] != QString()) {
            QToolTip::showText(helpEvent->globalPos(), m_tooltipForLine[row]);
        } else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
    return QPlainTextEdit::event(event);
}

void CodeEditor::enableBreakpointArea() {
    // Enables breakpoint area, and sets cursor for the breakpoint area to a "clickable" cursor
    m_breakpointAreaEnabled = true;
    m_breakpointArea->setCursor(Qt::PointingHandCursor);
}

void CodeEditor::updateSidebar(const QRect& rect, int dy) {
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
        m_breakpointArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
        m_breakpointArea->update(0, rect.y(), m_breakpointArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
        updateSidebarWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent* e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left() + m_breakpointArea->width(), cr.top(), lineNumberAreaWidth(), cr.height()));

    m_breakpointArea->setGeometry(cr.left(), cr.top(), m_breakpointArea->width(), cr.height());
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Colors::Medalist).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(Qt::lightGray).lighter(120));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(Qt::gray).darker(130));
            painter.drawText(0, top, m_lineNumberArea->width() - 3, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::breakpointAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_breakpointArea);

    // When caret flashes in QPlainTextEdit a paint event is sent to this widget,
    // with a height of a line in the edit. We override this paint event by always
    // redrawing the visible breakpoint area
    auto area = m_breakpointArea->rect();
    QLinearGradient gradient = QLinearGradient(area.topLeft(), area.bottomRight());
    if (m_breakpointAreaEnabled) {
        gradient.setColorAt(0, QColor(Colors::FoundersRock).lighter(120));
        gradient.setColorAt(1, QColor(Colors::FoundersRock));
    } else {
        gradient.setColorAt(0, QColor(Qt::lightGray).lighter(120));
        gradient.setColorAt(1, QColor(Qt::lightGray).lighter(120));
    }
    painter.fillRect(area, gradient);

    if (m_breakpointAreaEnabled) {
        QTextBlock block = firstVisibleBlock();
        uint32_t address = block.blockNumber() * ProcessorHandler::get()->currentISA()->bytes();
        int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + static_cast<int>(blockBoundingRect(block).height());

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                if (ProcessorHandler::get()->hasBreakpoint(address)) {
                    painter.drawPixmap(m_breakpointArea->padding, top, m_breakpointArea->imageWidth,
                                       m_breakpointArea->imageHeight, m_breakpointArea->m_breakpoint);
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + static_cast<int>(blockBoundingRect(block).height());
            address += ProcessorHandler::get()->currentISA()->bytes();
        }
    }
}

void CodeEditor::setupSyntaxHighlighter() {
    // Creates AsmHighlighter object and connects it to the current document
    m_highlighter = new SyntaxHighlighter(document());
    // connect tooltip changes from asm highlighter
    connect(m_highlighter, &SyntaxHighlighter::setTooltip, this, &CodeEditor::updateTooltip);

    // The highlighting is reset upon line count changes, to detect label invalidation
    connect(this->document(), &QTextDocument::cursorPositionChanged, m_highlighter,
            &SyntaxHighlighter::invalidateLabels);
    connect(this->document(), &QTextDocument::blockCountChanged, m_highlighter,
            &SyntaxHighlighter::clearAndRehighlight);
}

long CodeEditor::addressForPos(const QPoint& pos) const {
    if (!m_breakpointAreaEnabled)
        return -1;

    // Get line height
    QTextBlock block = firstVisibleBlock();

    const auto height = blockBoundingRect(block).height();

    // Find block index in the codeeditor
    int index;
    if (block == document()->findBlockByLineNumber(0)) {
        index = static_cast<int>((pos.y() - contentOffset().y()) / height);
    } else {
        index = static_cast<int>((pos.y() + contentOffset().y()) / height);
    }
    // Get actual block index
    while (index > 0) {
        block = block.next();
        index--;
    }
    if (!block.isValid())
        return -1;

    // Toggle breakpoint
    return block.blockNumber() * ProcessorHandler::get()->currentISA()->bytes();
}

bool CodeEditor::hasBreakpoint(const QPoint& pos) const {
    if (!m_breakpointAreaEnabled)
        return false;

    return ProcessorHandler::get()->hasBreakpoint(static_cast<unsigned>(addressForPos(pos)));
}

void CodeEditor::breakpointClick(const QPoint& pos) {
    if (m_breakpointAreaEnabled) {
        // Toggle breakpoint
        auto address = addressForPos(pos);
        if (!(address < 0)) {
            ProcessorHandler::get()->toggleBreakpoint(static_cast<unsigned>(address));
            repaint();
        }
    }
}

// -------------- breakpoint area ----------------------------------

BreakpointArea::BreakpointArea(CodeEditor* editor) : QWidget(editor) {
    codeEditor = editor;
}

void BreakpointArea::contextMenuEvent(QContextMenuEvent* event) {
    // setup context menu
    QMenu contextMenu;

    // Create and connect actions for removing and setting breakpoints
    auto* toggleAction = contextMenu.addAction("Toggle breakpoint");
    auto* removeAllAction = contextMenu.addAction("Remove all breakpoints");

    connect(toggleAction, &QAction::triggered, [=] { codeEditor->breakpointClick(event->pos()); });
    connect(removeAllAction, &QAction::triggered, [=] {
        codeEditor->clearBreakpoints();
        repaint();
    });

    contextMenu.exec(event->globalPos());
}
}  // namespace Ripes
