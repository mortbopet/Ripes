#include "codeeditor.h"
#include "defines.h"
#include "pipeline.h"

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

CodeEditor::CodeEditor(QWidget* parent) : QPlainTextEdit(parent) {
    m_lineNumberArea = new LineNumberArea(this);
    m_breakpointArea = new BreakpointArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateSidebarWidth(int)));

    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateSidebar(QRect, int)));

    updateSidebarWidth(0);

    // Set font for the entire widget. calls to fontMetrics() will get the
    // dimensions of the currently set font
    m_font = QFont("Monospace");  // set default font to Monospace on unix systems
    m_font.setStyleHint(QFont::Monospace);
    m_font.setPointSize(10);
    setFont(m_font);
    m_fontTimer.setSingleShot(true);

    // set event filter for catching scroll events
    installEventFilter(this);

    // needed for instant tooltip displaying
    setMouseTracking(true);

    setWordWrapMode(QTextOption::NoWrap);
}

void CodeEditor::setupAssembler() {
    // configures the change-timer and assembler connectivity with Parser
    m_changeTimer.setInterval(500);
    m_changeTimer.setSingleShot(true);
    // A change in the document will start the timer - when the timer elapses, the contents will be assembled if there
    // is no syntax error. By doing this, the timer is restartet each time a change occurs (ie. a user is continuously
    // typing)
    connect(this, &CodeEditor::textChanged, [=] {
        if (m_timerEnabled)
            m_changeTimer.start();
    });
    connect(&m_changeTimer, &QTimer::timeout, this, &CodeEditor::assembleCode);
    m_assembler = new Assembler();
}

void CodeEditor::assembleCode() {
    if (m_tooltipForLine.isEmpty()) {
        // No tooltips available => syntax is accepted

        const QByteArray& ret = m_assembler->assembleBinaryFile(*document());
        if (!m_assembler->hasError()) {
            emit assembledSuccessfully(ret, true, 0x0);
            if (m_assembler->hasData()) {
                emit assembledSuccessfully(m_assembler->getDataSegment(), false, DATASTART);
            }
        } else {
            QMessageBox err;
            err.setText("Error in assembling file.");
            err.exec();
        }
    }
    // Restart the simulator to trigger the data memory to be loaded into the main memory. Bad code that this is done
    // from here, but it works
    Pipeline::getPipeline()->restart();
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
    auto breakpoints = Pipeline::getPipeline()->getBreakpoints();
    breakpoints->clear();
}

void CodeEditor::updateBreakpoints() {
    // called after disassembler text has been set

    auto breakpoints = Pipeline::getPipeline()->getBreakpoints();
    // Remove breakpoints if a breakpoint line has been removed
    while (!breakpoints->empty() && *(breakpoints->rbegin()) > ((blockCount() - 1) * 4)) {  // byte indexed
        breakpoints->erase(std::prev(breakpoints->end()));
    }
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
    int bottom = top + (int)blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(Qt::gray).darker(130));
            painter.drawText(0, top, m_lineNumberArea->width() - 3, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
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
        int blockNumber = block.blockNumber() * 4;  // byte indexed
        int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
        int bottom = top + (int)blockBoundingRect(block).height();

        auto breakpoints = Pipeline::getPipeline()->getBreakpoints();

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                if (breakpoints->find(blockNumber) != breakpoints->end()) {
                    painter.drawPixmap(m_breakpointArea->padding, top, m_breakpointArea->imageWidth,
                                       m_breakpointArea->imageHeight, m_breakpointArea->m_breakpoint);
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + (int)blockBoundingRect(block).height();
            blockNumber += 4;
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

void CodeEditor::breakpointClick(QMouseEvent* event, int forceState) {
    if (m_breakpointAreaEnabled) {
        // Get line height
        QTextBlock block = firstVisibleBlock();
        auto height = blockBoundingRect(block).height();

        // Find block index in the codeeditor
        int index;
        if (block == document()->findBlockByLineNumber(0)) {
            index = (event->pos().y() - contentOffset().y()) / height;
        } else {
            index = (event->pos().y() + contentOffset().y()) / height;
        }
        // Get actual block index
        while (index > 0) {
            block = block.next();
            index--;
        }
        // Set or unset breakpoint
        // Since we want the simulator as fast as possible, the breakpoints are byte-indexed
        auto breakpoints = Pipeline::getPipeline()->getBreakpoints();
        int blockNumber = block.blockNumber() * 4;
        if (block.isValid()) {
            auto brkptIter = breakpoints->find(blockNumber);
            // Set/unset breakpoint
            if (forceState == 1) {
                breakpoints->insert(blockNumber);
            } else if (forceState == 2) {
                if (brkptIter != breakpoints->end())
                    breakpoints->erase(breakpoints->find(blockNumber));
            } else {
                if (brkptIter != breakpoints->end()) {
                    breakpoints->erase(brkptIter);
                } else {
                    breakpoints->insert(blockNumber);
                }
            }
            repaint();
        }
    }
}

// -------------- breakpoint area ----------------------------------

BreakpointArea::BreakpointArea(CodeEditor* editor) : QWidget(editor) {
    codeEditor = editor;

    // Create and connect actions for removing and setting breakpoints
    m_removeAction = new QAction("Remove breakpoint", this);
    m_removeAllAction = new QAction("Remove all breakpoints", this);
    m_addAction = new QAction("Add breakpoint", this);

    connect(m_removeAction, &QAction::triggered, [=] { codeEditor->breakpointClick(m_event, 2); });
    connect(m_addAction, &QAction::triggered, [=] { codeEditor->breakpointClick(m_event, 1); });
    connect(m_removeAllAction, &QAction::triggered, [=] {
        codeEditor->clearBreakpoints();
        repaint();
    });

    // Construct default mouseButtonEvent
    m_event = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
}

void BreakpointArea::contextMenuEvent(QContextMenuEvent* event) {
    // setup context menu
    QMenu contextMenu;

    // Translate event to a QMouseEvent in case add/remove single breakpoint is
    // triggered
    *m_event = QMouseEvent(QEvent::MouseButtonRelease, event->pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    contextMenu.addAction(m_addAction);
    contextMenu.addAction(m_removeAction);
    contextMenu.addAction(m_removeAllAction);

    contextMenu.exec(event->globalPos());
}
